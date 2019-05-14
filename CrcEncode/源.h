#pragma once

void ProcessDestinationAddr(std::fstream &outfile);

void MoveNewBitToCrc(unsigned char &crc, char temp, unsigned char i);

void CalculateCRCValue(int total, std::fstream &outfile, unsigned char &crc);

void ProcessSourceAddr(std::fstream &outfile);

void ProcessLengthAndData(std::fstream &outfile);
