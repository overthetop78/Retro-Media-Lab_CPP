#pragma once

#include "EmulationExport.h"
#include <QList>
#include <QObject>
#include <QString>
#include <QUuid>
#include <functional>
#include <memory>
#include <optional>

namespace RetroMediaLab::Emulation {
struct Configuration;

enum class NetworkAddressType {
    None = 0,
    Ipv4 = 1,
    Ipv6 = 2,
    Mac48 = 3,
    Mac64 = 4,
    EconetStation = 5,
    AppleTalkNode = 6,
    IpxNode = 7,
    DecnetNode = 8,
    X121 = 9,
    ArpanetHostImp = 10,
    ArcnetNode = 11,
    OmninetNode = 12,
    SinclairStation = 13,
    DeviceAddress = 14,
};

enum class NetworkCompatibility {
    NotEvaluated = 0,
    Compatible = 1,
    SameMachine = 2,
    MachineOffline = 3,
    InterfaceDisconnected = 4,
    NetworkMismatch = 5,
    FormatMismatch = 6,
    TransportMismatch = 7,
    NativeParametersMismatch = 8,
    AddressConflict = 9,
    InterfaceMismatch = 10,
};

enum class NetworkInterfaceType {
    None = 0,
    Ethernet = 1,
    Wireless = 2,
    TokenRing = 3,
    TokenBus = 4,
    Arcnet = 5,
    Fddi = 6,
    LocalTalk = 7,
    Econet = 8,
    SerialPort = 9,
    ParallelPort = 10,
    Modem = 11,
    Isdn = 12,
    X25 = 13,
    Atm = 14,
    FrameRelay = 15,
    Arpanet1822 = 16,
    Omninet = 17,
    SinclairNetwork = 18,
    FibreChannel = 19,
    InfiniBand = 20,
    FireWire = 21,
    UsbNetwork = 22,
    Infrared = 23,
    Bluetooth = 24,
    GameLink = 25,
    Midi = 26,
    SharedPeripheral = 27,
};

enum class NetworkProtocol {
    None = 0,
    Unknown = 1,
    EthernetII = 2,
    Ieee802Llc = 3,
    Ieee802Snap = 4,
    Arp = 5,
    Rarp = 6,
    Econet = 7,
    Aun = 8,
    LocalTalkLlap = 9,
    AppleTalkDdp = 10,
    Ipv4 = 11,
    Ipv6 = 12,
    Ipx = 13,
    Spx = 14,
    Tcp = 15,
    Udp = 16,
    Icmp = 17,
    IcmpV6 = 18,
    Igmp = 19,
    Mld = 20,
    Slip = 21,
    Cslip = 22,
    Ppp = 23,
    Lcp = 24,
    Ipcp = 25,
    Ipv6Cp = 26,
    Ipxcp = 27,
    Atcp = 28,
    Hdlc = 29,
    Lapb = 30,
    X25 = 31,
    Ddcmp = 32,
    Decnet = 33,
    Sna = 34,
    Sdlc = 35,
    Bsc = 36,
    Ncp = 37,
    XnsIdp = 38,
    Pup = 39,
    Chaosnet = 40,
    Clnp = 41,
    Ax25 = 42,
    Omninet = 43,
    SinclairNetwork = 44,
    GameLink = 45,
};

enum class NetworkRelayType {
    None = 0,
    Repeater = 1,
    Hub = 2,
    Switch = 3,
    Bridge = 4,
    Router = 5,
    Gateway = 6,
    Proxy = 7,
    Nat = 8,
    PeripheralMultiplexer = 9,
};

enum class NetworkTransferFormat {
    None = 0,
    EthernetIIFrame = 1,
    Ieee802LlcFrame = 2,
    Ieee802SnapFrame = 3,
    TokenRingFrame = 4,
    TokenBusFrame = 5,
    ArcnetFrame = 6,
    FddiFrame = 7,
    LocalTalkFrame = 8,
    EconetFrame = 9,
    AunPacket = 10,
    PppFrame = 11,
    SlipPacket = 12,
    HdlcFrame = 13,
    LapbFrame = 14,
    X25Packet = 15,
    FrameRelayFrame = 16,
    AtmCell = 17,
    Arpanet1822Message = 18,
    Arpanet1822LMessage = 19,
    OmninetFrame = 20,
    SinclairNetworkFrame = 21,
    Ieee80211Frame = 22,
    FibreChannelFrame = 23,
    InfiniBandPacket = 24,
    SerialBitStream = 25,
    SerialByteStream = 26,
    ParallelByteStream = 27,
    GameLinkBitStream = 28,
    MidiByteStream = 29,
    PeripheralBus = 30,
    Ipv4Packet = 31,
    Ipv6Packet = 32,
};

enum class NetworkTransport {
    None = 0,
    InProcess = 1,
    Udp = 2,
    Tcp = 3,
    Ipx = 4,
    NamedPipe = 5,
    UnixSocket = 6,
    SharedMemory = 7,
    SerialPort = 8,
    Tap = 9,
    Pcap = 10,
};

enum class NetworkType {
    None = 0,
    Ethernet = 1,
    WirelessLan = 2,
    TokenRing = 3,
    TokenBus = 4,
    Arcnet = 5,
    Fddi = 6,
    LocalTalk = 7,
    Econet = 8,
    Arpanet = 9,
    Omninet = 10,
    SinclairNetwork = 11,
    SerialNetwork = 12,
    ParallelNetwork = 13,
    X25 = 14,
    Isdn = 15,
    Atm = 16,
    FrameRelay = 17,
    FibreChannel = 18,
    InfiniBand = 19,
    FireWire = 20,
    UsbNetwork = 21,
    Infrared = 22,
    BluetoothPan = 23,
    GameLink = 24,
    Midi = 25,
    SharedPeripheral = 26,
};


struct NetworkAddress {
    NetworkAddressType type = NetworkAddressType::None;
    QString value;
    bool operator==(const NetworkAddress&) const = default;
};
struct NetworkRelay {
    NetworkRelayType type = NetworkRelayType::None;
    QString target;
    bool operator==(const NetworkRelay&) const = default;
};
struct NetworkEndpoint {
    QString moduleId;
    QUuid configurationId;
    QString machineId;
    QString interfaceId;
    NetworkInterfaceType interfaceType = NetworkInterfaceType::None;
    NetworkType network = NetworkType::None;
    QList<NetworkProtocol> protocols;
    NetworkTransferFormat transferFormat = NetworkTransferFormat::None;
    NetworkTransport transport = NetworkTransport::None;
    std::optional<QString> ipAddress;
    std::optional<int> port;
    std::optional<QString> hostEndpoint;
    QList<NetworkAddress> addresses;
    QList<NetworkRelay> relays;
    bool isRunning = false;
    bool operator==(const NetworkEndpoint&) const = default;
};

class IEmulationNetworkManager {
public:
    virtual ~IEmulationNetworkManager() = default;
    virtual QList<NetworkEndpoint> describeNetworks(const Configuration&) const = 0;
};

class NetworkRegistration {
public:
    virtual ~NetworkRegistration() = default;
    virtual void update(NetworkEndpoint) = 0;
};

class RETROMEDIALAB_EMULATION_API NetworkRegistry final : public QObject {
    Q_OBJECT
public:
    using NativeRule = std::function<NetworkCompatibility(const NetworkEndpoint&)>;

    explicit NetworkRegistry(QObject* parent = nullptr);
    ~NetworkRegistry() override;
    quint64 version() const;
    QList<NetworkEndpoint> endpoints() const;
    void setConfigurations(const QString& moduleId, QList<NetworkEndpoint>);
    std::unique_ptr<NetworkRegistration> registerInterface(NetworkEndpoint, NativeRule);
    NetworkCompatibility compatibility(const NetworkEndpoint&, const NetworkEndpoint&) const;
    QList<NetworkEndpoint> peers(const NetworkEndpoint&) const;

Q_SIGNALS:
    void changed();

private:
    struct Data;
    struct Registration;
    std::shared_ptr<Data> data_;
};

RETROMEDIALAB_EMULATION_API NetworkRegistry& networks();
RETROMEDIALAB_EMULATION_API QString networkResourceKey(NetworkAddressType);
RETROMEDIALAB_EMULATION_API QString networkResourceKey(NetworkCompatibility);
RETROMEDIALAB_EMULATION_API QString networkResourceKey(NetworkInterfaceType);
RETROMEDIALAB_EMULATION_API QString networkResourceKey(NetworkProtocol);
RETROMEDIALAB_EMULATION_API QString networkResourceKey(NetworkRelayType);
RETROMEDIALAB_EMULATION_API QString networkResourceKey(NetworkTransferFormat);
RETROMEDIALAB_EMULATION_API QString networkResourceKey(NetworkTransport);
RETROMEDIALAB_EMULATION_API QString networkResourceKey(NetworkType);
}
