#pragma once
#include <string_view>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::AtariTapeConstants {
inline constexpr std::string_view FormatId="tape.atari-cas",WriterId="tape-atari-cas",Extension=".cas",DecoderId="atari-tape",MachineId="atari-8bit",FileMarker="FUJI",BaudChunk="baud",DataChunk="data",FskChunk="fsk ";
inline constexpr std::string_view ChunkKey="chunkId",AuxKey="auxiliary",BaudKey="baudRate",NameKey="name",InternalNameKey="internalName",ChunkCountKey="chunkCount",ChunkTypesKey="chunkTypes",DataCountKey="dataChunkCount",FskCountKey="fskChunkCount",BaudRatesKey="baudRates",SystemKey="systemId",DefaultBaudKey="defaultBaudRate",SignalKey="signal",ChunkOffsetKey="chunkOffset",Mark="mark",Space="space",ListSeparator=", ",NameSeparator="; ";
inline constexpr int HeaderSize=8,IdentifierSize=4,LengthOffset=4,AuxOffset=6,WordBytes=2,DefaultBaud=600,SerialBits=8,StopBits=1,FrameBits=10,SpaceFrequency=3995,MarkFrequency=5327,MinimumSamplesPerBit=8,MaximumChunkSize=65535,MaximumAscii=127;
inline constexpr double FskUnitMilliseconds=0.1,TicksPerMillisecond=10000;
inline constexpr char Null='\0',Blank=' ',AsciiReplacement='?',FirstDigit='0',LastDigit='9';
inline constexpr int StandardRecordLength=132,RecordDataOffset=3,RecordDataLength=128,PartialLengthOffset=130,ChecksumOffset=131,MinimumRecordLength=4,ChecksumMask=255,ChecksumShift=8;
inline constexpr std::uint8_t RecordSync=0x55,FullRecord=0xfc,PartialRecord=0xfa,EndRecord=0xfe;
inline constexpr std::string_view SourceKindKey="sourceKind",LogicalOrdinalKey="logicalFileOrdinal",RecordCountKey="recordCount",FullCountKey="fullRecordCount",PartialCountKey="partialRecordCount",EndPresentKey="endRecordPresent",ChecksumPresentKey="checksumPresent",RecordsSource="atari-cas-records",CasFskSource="cas-fsk",WaveFskSource="wav-fsk",True="True",False="False",WaveFormat="tape.wav",CswFormat="tape.csw",RawFormat="tape.raw";
}
