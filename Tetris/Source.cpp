#pragma once 
#include <iostream>
#include <thread>
#include <vector>

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <Windows.h>
using namespace std;

wstring tetronimo[7];
int nFieldWidth = 12; 
int nFieldHeight = 18;
unsigned char* pField = nullptr;

int nScreenWidth = 120;
int nScreenHeight = 180;

int Rotate(int px, int py, int r) {
	switch (r % 4)
	{
	case 0: return py * 4 + px;
	case 1: return 12 + py - (px * 4);
	case 2: return 15 - (py * 4) - px;
	case 3: return 3 - py + (px * 4);
	default:
		return 0;
	}
}

bool DoesPieceFit(int nTetronimo, int nRotation, int nPosx, int nPosY) {
	for(int px = 0; px <4; px++)
		for (int py = 0; py < 4; py++) {
			//get index into pice 
			int pi = Rotate(px, py, nRotation);

			//get index in field
			int fi = (nPosY + py) * nFieldWidth + (nPosx+px);

			if (nPosx + px >= 0 && nPosx + px < nFieldWidth) {
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
					if (tetronimo[nTetronimo][pi] == L'X' && pField[fi] != 0)
						return false;
				}
			}
		}

	return true;
}

int main() {

	//create assets 
	tetronimo[0].append(L"..X.");
	tetronimo[0].append(L"..X.");
	tetronimo[0].append(L"..X.");
	tetronimo[0].append(L"..X.");

	tetronimo[1].append(L"..X.");
	tetronimo[1].append(L".XX.");
	tetronimo[1].append(L".X..");
	tetronimo[1].append(L"....");

	tetronimo[2].append(L".X..");
	tetronimo[2].append(L".XX.");
	tetronimo[2].append(L"..X.");
	tetronimo[2].append(L"....");

	tetronimo[3].append(L"....");
	tetronimo[3].append(L".XX.");
	tetronimo[3].append(L".XX.");
	tetronimo[3].append(L"....");

	tetronimo[4].append(L"..X.");
	tetronimo[4].append(L".XX.");
	tetronimo[4].append(L"..X.");
	tetronimo[4].append(L"....");

	tetronimo[5].append(L"....");
	tetronimo[5].append(L".XX.");
	tetronimo[5].append(L"..X.");
	tetronimo[5].append(L"..X.");

	tetronimo[6].append(L"....");
	tetronimo[6].append(L".XX.");
	tetronimo[6].append(L".X..");
	tetronimo[6].append(L".X..");

	pField = new unsigned char[nFieldHeight * nFieldWidth];
	for (int x = 0; x < nFieldWidth; x++) {
		for (int y = 0; y < nFieldHeight; y++) {
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
		}
	}

	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	bool bGameOver = false;

	//game state
	int nCurrentPiece = 0;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;

	bool bKey[4];
	bool bRotateHold = false;

	int nSpeed = 20;
	int nSpeedCounter = 0;
	bool bForcedDown = false;
	int nPieceCount = 0;
	int nScore = 0;

	vector<int> vLines;

	while (!bGameOver) {

		//Timing
		this_thread::sleep_for(16ms);
		nSpeedCounter++;
		bForcedDown = (nSpeedCounter == nSpeed);

		//Input 
		for (int k = 0; k < 4; k++) {							  //R //L //D Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		}

		//Logic
		nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

		if (bKey[3]) {
			nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = true;
		}
		else {
			bRotateHold = false;
		}

		if (bForcedDown) {
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
				nCurrentY++;
			}
			else {
				//lock the piece 
				for(int px = 0; px<4; px++)
					for (int py = 0; py < 4; py++) {
						if (tetronimo[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X') {
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
						}
					}

				nPieceCount++;
				if (nPieceCount % 10 == 0)
					if (nSpeed >= 10) nSpeed--;

				//check if we got lines 
				for(int py = 0;  py < 4; py++)
					if (nCurrentY + py < nFieldHeight - 1) {
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++) 
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px] != 0);

							if (bLine) {
								for (int px = 1; px < nFieldWidth - 1; px++) {
									pField[(nCurrentY + py) * nFieldWidth + px] = 8;
								}

								vLines.push_back(nCurrentY + py);
							}
						
					}

				nScore + 25;
				if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;

				//choose the next piece
				nCurrentX = nFieldWidth/2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;
				//check for gameover
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}

			nSpeedCounter = 0;
		}

		//render output

		// draw field 
		for (int x = 0; x < nFieldWidth; x++) {
			for (int y = 0; y < nFieldHeight; y++) {
				screen[(y) * nScreenWidth + (x)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
			}
		}

		//draw current piece
		for(int px =0; px<4;px++)
			for (int py = 0; py < 4; py++) {
				if (tetronimo[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
					screen[(nCurrentY + py) * nScreenWidth + (nCurrentX + px)] = nCurrentPiece + 65;
			}

		//draw score 
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

		if (!vLines.empty()) {
			//Display frame
			WriteConsoleOutputCharacter(hConsole, screen, nScreenHeight * nScreenWidth, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);

			for (auto& v : vLines) {
				for (int px = 1; px < nFieldWidth - 1; px++) {

					for (int py = v; py > 0; py--) {
						pField[py*nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					}
					//pField = 0;
				}
			}
			vLines.clear();
		}

		WriteConsoleOutputCharacter(hConsole, screen, nScreenHeight * nScreenWidth, { 0,0 }, &dwBytesWritten);
	}

	CloseHandle(hConsole);
	cout << "Game Over! Score: " << nScore << endl;
	system("pause");
	return 0;
}