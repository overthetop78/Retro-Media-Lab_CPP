#include <RetroMediaLab/Emulation/Networks.h>

#include <QHostAddress>
#include <algorithm>
#include <array>
#include <map>
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace RetroMediaLab::Emulation {
namespace {
using Identity = std::tuple<QString, QUuid, QString>;
using Compatibility = NetworkCompatibility;
constexpr int MinimumPort = 1, MaximumPort = 65535;
constexpr char ResourcePrefix[] = "Emulation.Network.Value.";
constexpr char InvalidEndpoint[] = "Emulation.Network.Error.InvalidEndpoint";
constexpr char DuplicateInterface[] = "Emulation.Network.Error.DuplicateInterface";
constexpr char InactiveRegistration[] = "Emulation.Network.Error.InactiveRegistration";
constexpr char InvalidValue[] = "Emulation.Network.Error.InvalidValue";

Identity identity(const NetworkEndpoint& endpoint) {
    return {endpoint.moduleId, endpoint.configurationId, endpoint.interfaceId};
}

NetworkEndpoint freeze(NetworkEndpoint endpoint, bool running) {
    if (endpoint.port && (*endpoint.port < MinimumPort || *endpoint.port > MaximumPort))
        throw std::invalid_argument(InvalidEndpoint);
    if (endpoint.ipAddress) {
        const QHostAddress ip(*endpoint.ipAddress);
        if (ip.isNull()) throw std::invalid_argument(InvalidEndpoint);
        endpoint.ipAddress = ip.toString();
    }
    endpoint.isRunning = running;
    return endpoint;
}

bool disconnected(const NetworkEndpoint& endpoint) {
    return endpoint.interfaceType == NetworkInterfaceType::None || endpoint.network == NetworkType::None ||
        endpoint.transferFormat == NetworkTransferFormat::None || endpoint.transport == NetworkTransport::None;
}

template<class Enum, std::size_t Size>
QString resourceKey(Enum value, const std::array<const char*, Size>& names) {
    const auto index = static_cast<std::size_t>(value);
    if (index >= names.size()) throw std::out_of_range(InvalidValue);
    return QLatin1String(ResourcePrefix) + QLatin1String(names[index]);
}
}

struct NetworkRegistry::Data {
    struct Active {
        QUuid token;
        NetworkEndpoint endpoint;
        std::shared_ptr<const NativeRule> rule;
    };
    std::mutex mutex;
    std::map<Identity, NetworkEndpoint> configured;
    std::map<Identity, Active> active;
    QList<NetworkEndpoint> snapshot;
    quint64 version = 0;

    void publish() {
        auto combined = configured;
        for (const auto& [key, entry] : active)
            combined.insert_or_assign(key, entry.endpoint);
        QList<NetworkEndpoint> next;
        next.reserve(qsizetype(combined.size()));
        for (auto& [key, endpoint] : combined)
            next.append(std::move(endpoint));
        std::stable_sort(next.begin(), next.end(), [](const auto& a, const auto& b) {
            return std::tie(a.moduleId, a.machineId, a.interfaceId) < std::tie(b.moduleId, b.machineId, b.interfaceId);
        });
        snapshot = std::move(next);
        ++version;
    }
};

struct NetworkRegistry::Registration final : NetworkRegistration {
    NetworkRegistry& owner;
    std::weak_ptr<Data> data;
    Identity key;
    QUuid token;

    Registration(NetworkRegistry& owner, const std::shared_ptr<Data>& data, Identity key, QUuid token)
        : owner(owner), data(data), key(std::move(key)), token(token) {}
    ~Registration() override {
        try {
            if (const auto state = data.lock()) {
                decltype(state->active)::node_type removed;
                {
                    const std::lock_guard lock(state->mutex);
                    const auto current = state->active.find(key);
                    if (current == state->active.end() || current->second.token != token) return;
                    removed = state->active.extract(current);
                    state->publish();
                }
                Q_EMIT owner.changed();
            }
        } catch (...) {
            // A notification must not interrupt session teardown.
        }
    }
    void update(NetworkEndpoint endpoint) override {
        if (identity(endpoint) != key) throw std::invalid_argument(InvalidEndpoint);
        endpoint = freeze(std::move(endpoint), true);
        const auto state = data.lock();
        if (!state) throw std::logic_error(InactiveRegistration);
        {
            const std::lock_guard lock(state->mutex);
            const auto current = state->active.find(key);
            if (current == state->active.end() || current->second.token != token)
                throw std::logic_error(InactiveRegistration);
            current->second.endpoint = std::move(endpoint);
            state->publish();
        }
        Q_EMIT owner.changed();
    }
};

NetworkRegistry::NetworkRegistry(QObject* parent) : QObject(parent), data_(std::make_shared<Data>()) {}
NetworkRegistry::~NetworkRegistry() = default;

quint64 NetworkRegistry::version() const {
    const std::lock_guard lock(data_->mutex);
    return data_->version;
}

QList<NetworkEndpoint> NetworkRegistry::endpoints() const {
    const std::lock_guard lock(data_->mutex);
    return data_->snapshot;
}

void NetworkRegistry::setConfigurations(const QString& moduleId, QList<NetworkEndpoint> descriptions) {
    std::map<Identity, NetworkEndpoint> replacement;
    for (auto& endpoint : descriptions) {
        if (endpoint.moduleId != moduleId) throw std::invalid_argument(InvalidEndpoint);
        const auto key = identity(endpoint);
        if (!replacement.emplace(key, freeze(std::move(endpoint), false)).second)
            throw std::invalid_argument(DuplicateInterface);
    }
    {
        const std::lock_guard lock(data_->mutex);
        std::erase_if(data_->configured, [&](const auto& entry) { return std::get<0>(entry.first) == moduleId; });
        data_->configured.merge(replacement);
        data_->publish();
    }
    Q_EMIT changed();
}

std::unique_ptr<NetworkRegistration> NetworkRegistry::registerInterface(NetworkEndpoint endpoint, NativeRule rule) {
    if (!rule) throw std::invalid_argument(InvalidEndpoint);
    endpoint = freeze(std::move(endpoint), true);
    const auto key = identity(endpoint);
    const auto token = QUuid::createUuid();
    auto registration = std::make_unique<Registration>(*this, data_, key, token);
    const auto callback = std::make_shared<const NativeRule>(std::move(rule));
    {
        const std::lock_guard lock(data_->mutex);
        if (!data_->active.try_emplace(key, Data::Active{token, std::move(endpoint), callback}).second)
            throw std::logic_error(DuplicateInterface);
        data_->publish();
    }
    Q_EMIT changed();
    return registration;
}

NetworkCompatibility NetworkRegistry::compatibility(const NetworkEndpoint& first, const NetworkEndpoint& second) const {
    if (first.moduleId == second.moduleId && first.configurationId == second.configurationId)
        return Compatibility::SameMachine;
    Data::Active local, remote;
    {
        const std::lock_guard lock(data_->mutex);
        const auto a = data_->active.find(identity(first)), b = data_->active.find(identity(second));
        if (a == data_->active.end() || b == data_->active.end()) return Compatibility::MachineOffline;
        local = a->second;
        remote = b->second;
    }
    if (disconnected(local.endpoint) || disconnected(remote.endpoint)) return Compatibility::InterfaceDisconnected;
    if (local.endpoint.network != remote.endpoint.network) return Compatibility::NetworkMismatch;
    if (local.endpoint.interfaceType != remote.endpoint.interfaceType) return Compatibility::InterfaceMismatch;
    if (local.endpoint.transferFormat != remote.endpoint.transferFormat) return Compatibility::FormatMismatch;
    if (local.endpoint.transport != remote.endpoint.transport) return Compatibility::TransportMismatch;
    const auto result = (*local.rule)(remote.endpoint);
    return result == Compatibility::Compatible ? (*remote.rule)(local.endpoint) : result;
}

QList<NetworkEndpoint> NetworkRegistry::peers(const NetworkEndpoint& endpoint) const {
    auto result = endpoints();
    result.removeIf([&](const auto& peer) {
        return !peer.isRunning || compatibility(endpoint, peer) != Compatibility::Compatible;
    });
    return result;
}

NetworkRegistry& networks() {
    static NetworkRegistry registry;
    return registry;
}

QString networkResourceKey(NetworkAddressType value) {
    constexpr std::array names{
        "None", "Ipv4", "Ipv6", "Mac48",
        "Mac64", "Econet", "AppleTalkDdp", "Ipx",
        "Decnet", "X121", "Arpanet", "Arcnet",
        "Omninet", "SinclairNetwork", "DeviceAddress",
    };
    return resourceKey(value, names);
}

QString networkResourceKey(NetworkCompatibility value) {
    constexpr std::array names{
        "NotEvaluated", "Compatible", "SameMachine", "MachineOffline",
        "InterfaceDisconnected", "NetworkMismatch", "FormatMismatch", "TransportMismatch",
        "NativeParametersMismatch", "AddressConflict", "InterfaceMismatch",
    };
    return resourceKey(value, names);
}

QString networkResourceKey(NetworkInterfaceType value) {
    constexpr std::array names{
        "None", "Ethernet", "WirelessLan", "TokenRing",
        "TokenBus", "Arcnet", "Fddi", "LocalTalk",
        "Econet", "SerialPort", "ParallelPort", "Modem",
        "Isdn", "X25", "Atm", "FrameRelay",
        "Arpanet1822", "Omninet", "SinclairNetwork", "FibreChannel",
        "InfiniBand", "FireWire", "UsbNetwork", "Infrared",
        "Bluetooth", "GameLink", "Midi", "SharedPeripheral",
    };
    return resourceKey(value, names);
}

QString networkResourceKey(NetworkProtocol value) {
    constexpr std::array names{
        "None", "Unknown", "EthernetII", "Ieee802Llc",
        "Ieee802Snap", "Arp", "Rarp", "Econet",
        "Aun", "LocalTalkLlap", "AppleTalkDdp", "Ipv4",
        "Ipv6", "Ipx", "Spx", "Tcp",
        "Udp", "Icmp", "IcmpV6", "Igmp",
        "Mld", "Slip", "Cslip", "Ppp",
        "Lcp", "Ipcp", "Ipv6Cp", "Ipxcp",
        "Atcp", "Hdlc", "Lapb", "X25",
        "Ddcmp", "Decnet", "Sna", "Sdlc",
        "Bsc", "Ncp", "XnsIdp", "Pup",
        "Chaosnet", "Clnp", "Ax25", "Omninet",
        "SinclairNetwork", "GameLink",
    };
    return resourceKey(value, names);
}

QString networkResourceKey(NetworkRelayType value) {
    constexpr std::array names{
        "None", "Repeater", "Hub", "Switch",
        "Bridge", "Router", "Gateway", "Proxy",
        "Nat", "PeripheralMultiplexer",
    };
    return resourceKey(value, names);
}

QString networkResourceKey(NetworkTransferFormat value) {
    constexpr std::array names{
        "None", "EthernetII", "Ieee802Llc", "Ieee802Snap",
        "TokenRing", "TokenBus", "Arcnet", "Fddi",
        "LocalTalkLlap", "Econet", "Aun", "Ppp",
        "Slip", "Hdlc", "Lapb", "X25",
        "FrameRelay", "Atm", "Arpanet1822", "Arpanet1822LMessage",
        "Omninet", "SinclairNetwork", "WirelessLan", "FibreChannel",
        "InfiniBand", "SerialBitStream", "SerialByteStream", "ParallelByteStream",
        "GameLink", "Midi", "PeripheralBus", "Ipv4",
        "Ipv6",
    };
    return resourceKey(value, names);
}

QString networkResourceKey(NetworkTransport value) {
    constexpr std::array names{
        "None", "InProcess", "Udp", "Tcp",
        "Ipx", "NamedPipe", "UnixSocket", "SharedMemory",
        "SerialPort", "Tap", "Pcap",
    };
    return resourceKey(value, names);
}

QString networkResourceKey(NetworkType value) {
    constexpr std::array names{
        "None", "Ethernet", "WirelessLan", "TokenRing",
        "TokenBus", "Arcnet", "Fddi", "LocalTalk",
        "Econet", "Arpanet", "Omninet", "SinclairNetwork",
        "SerialNetwork", "ParallelNetwork", "X25", "Isdn",
        "Atm", "FrameRelay", "FibreChannel", "InfiniBand",
        "FireWire", "UsbNetwork", "Infrared", "BluetoothPan",
        "GameLink", "Midi", "SharedPeripheral",
    };
    return resourceKey(value, names);
}

}
