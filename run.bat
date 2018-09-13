echo off 
rem g++ Main.cpp StreamReader.cpp CodeBlock.cpp EntropyDecoder.cpp InverseWaveletTransform.cpp MetadataReader.cpp MQDecoder.cpp PacketDecoder.cpp PacketHeaderReader.cpp Subband.cpp TagTreeDecoder.cpp -o decompress

decompress %1 out1.txt

IF [%2] == [] GOTO withNoFile
python display_j2k.py %2
GOTO end
:withNoFile
python display_j2k.py
:end
echo on