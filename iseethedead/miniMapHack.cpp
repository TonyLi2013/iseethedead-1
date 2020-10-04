/*
	无法精确的找到大地图转换小地图的方法
	暂时（永久）停用

	+3BD740 updateMiniMap
*/
#include "pch.h"
#include "miniMapHack.h"

inline float GetUnitX(void* unit) {
	return *(float*)((unsigned int)unit + 0x284);
};
inline float GetUnitY(void* unit) {
	return *(float*)((unsigned int)unit + 0x288);
};

static uint32_t miniMapcallback, _W3XConversion;
static MiniMapHack::convert1 addrConvertCoord1;
static MiniMapHack::convert2 addrConvertCoord2;
MiniMapHack::MiniMapHack() {
	miniMapBackup = (uint32_t*)new uint32_t[255][255];
	miniMapcallback = gameDll + 0x771125;
	_W3XConversion = gameDll + 0xBB82BC;
	addrConvertCoord1 = (convert1)(gameDll + 0x528B0);
	addrConvertCoord2 = (convert2)(gameDll + 0x52F30);
	PlantDetourCall((BYTE*)gameDll + 0x3BE4F1, (BYTE*)DetourUpdateMiniMap, 5);
}

MiniMapHack::~MiniMapHack() {
	delete[]miniMapBackup;
}

int sub_6F052B50(unsigned int c)
{
	bool v1; // zf
	int v2; // edx
	int result; // eax
	v1 = !_BitScanReverse((unsigned long*)&v2, c);
	if (v1)
		result = 32;
	else
		result = 31 - v2;
	return result;
}

signed int* MiniMapHack::ConvertCoord1(signed int* a1, signed int* a2, DWORD* a3)
{
	signed int v3; // edx
	signed int* v4; // esi
	signed int v5; // eax
	int v6; // ebx
	int v7; // edx
	int v8; // edi
	int v9; // ecx
	unsigned int v10; // ecx
	int v12; // edi
	int v13; // eax
	int v14; // edi
	signed int v15; // [esp+Ch] [ebp-4h]
	int v16; // [esp+18h] [ebp+8h]
	unsigned int v17; // [esp+18h] [ebp+8h]

	v3 = *a2;
	v4 = a1;
	v15 = v3;
	v5 = *a3 ^ 0x80000000;
	v16 = v3 & 0x7F800000;
	if (!(v3 & 0x7F800000))
		goto LABEL_14;
	v6 = v5 & 0x7F800000;
	if (v5 & 0x7F800000)
	{
		v7 = ((v3 >> 31) ^ 2 * (v3 & 0x7FFFFF | 0x800000)) - (v3 >> 31);
		v8 = ((v5 >> 31) ^ 2 * (v5 & 0x7FFFFF | 0x800000)) - (v5 >> 31);
		v9 = v6 - v16;
		if (v6 - v16 > 0)
		{
			if (v9 < 192937984)
			{
				v7 >>= (unsigned int)v9 >> 23;
			LABEL_8:
				if (!(v8 + v7))
				{
					*v4 = 0;
					return v4;
				}
				v17 = (v8 + v7) & 0x80000000;
				v12 = abs(v8 + v7);
				v13 = sub_6F052B50(v12);
				if (8 - v13 < 0)
					v14 = v12 << (v13 - 8);
				else
					v14 = v12 >> (8 - v13);
				v5 = v17 | v14 & 0x7FFFFF | (v6 + ((8 - v13 - 1) << 23));
			}
		LABEL_14:
			*v4 = v5;
			return v4;
		}
		if (v9 > -192937984)
		{
			v10 = v16 - v6;
			v6 = v16;
			v8 >>= v10 >> 23;
			goto LABEL_8;
		}
		v3 = v15;
	}
	*v4 = v3;
	return v4;
}

uint32_t MiniMapHack::ConvertCoord2(int* a) {
	int v1;
	unsigned int v2; // eax
	unsigned int v4; // edx
	int v5; // esi
	unsigned int v6; // edx

	v1 = *a;
	v2 = ((unsigned int)*a >> 23) & 0xFF;
	if (v2 < 0x7F)
		return 0;
	v4 = v1 & 0x7FFFFF | 0x800000;
	if ((signed int)(150 - v2) < 0)
	{
		v5 = v1 >> 31;
		v6 = v4 << (v2 + 106);
	}
	else
	{
		v5 = v1 >> 31;
		v6 = v4 >> (-106 - v2);
	}
	return (v6 ^ v5) - v5;
}

uint32_t MiniMapHack::CoordToMinimap(float Loc, DWORD offst)
{
	float Result = 0;
	uint32_t conversion = *(uint32_t*)(_W3XConversion);
	ConvertCoord1((signed int*)&Result, (signed int*)&Loc, (DWORD*)(conversion + offst));
	DWORD t1 = *(DWORD*)(&Result);
	int t2 = t1 - 0x3000000;
	t2 ^= t1;
	DWORD t3 = t1 - 0x2800000;
	t2 >>= 0x1f;
	t2 = ~t2;
	t2 = t2 & t3;
	return ConvertCoord2((int *)&t2);
}

void MiniMapHack::ConvertMmap(MmapLoc& loc)
{
	int X = loc.X / 4, Y = loc.Y / 4;
	X = max(X, minX); X = min(X, maxX);
	Y = max(Y, minY); Y = min(Y, maxY);
	X -= minX; Y -= minY;
	float fX = X, fY = Y;
	fX = fX * xMult + unk;
	fY = fY * yMult + unk;
	__asm
	{
		MOV EAX, fX
		SHR EAX, 0xE
		MOVZX EAX, AL
		MOV X, EAX
		MOV EAX, fY
		SHR EAX, 0xE
		MOVZX EAX, AL
		MOV Y, EAX
	};
	X += unk2 - 2;
	Y = 0x100 - Y - unk3 - 2;

	loc.X = X;
	loc.Y = Y;
}

MmapLoc MiniMapHack::LocationToMinimap(float x, float y)
{
	MmapLoc ret;
	ret.X = CoordToMinimap(x, 0x6C);
	ret.Y = CoordToMinimap(y, 0x70);
	ConvertMmap(ret);
	return ret;
}

inline void MiniMapHack::draw_line(int x0, int y0, int x1, int y1, uint32_t c) {
	// Bresenham's line algorithm
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2, e2;
	while (true) {
		DrawPixel(x0 , y0, c);
		DrawPixel(x0 + 1 , y0, c);
		DrawPixel(x0 + 1 , y0 - 1, c);
		DrawPixel(x0 , y0 - 1, c);
		if (x0 == x1 && y0 == y1) break;
		e2 = err;
		if (e2 > -dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
}

void MiniMapHack::draw_unit(void* unit, Unit& obj)
{
	MmapLoc ret;
	ret.X = CoordToMinimap(GetUnitX(unit), 0x6C);
	ret.Y = CoordToMinimap(GetUnitY(unit), 0x70);
	ConvertMmap(ret);
	DrawPixel(ret.X, ret.Y, obj.color);
	DrawPixel(ret.X + 1, ret.Y, obj.color);
	DrawPixel(ret.X - 1, ret.Y, obj.color);
	DrawPixel(ret.X - 1, ret.Y + 1, obj.color);
	DrawPixel(ret.X - 1, ret.Y - 1, obj.color);
}

void MiniMapHack::DrawPixel(int x, int y, uint32_t color)
{
	size_t index = 0;
	if (x > 255 || x < 0) return;
	if (y > (255 - unk3) || y < 0) return;

	int offset = (y << 8) + x;
	//restore[index] = true;
	gameMiniMap[offset] = color;
}

void MiniMapHack::addLine(void* unit, float x, float y, unsigned int color) {
	MmapLoc to = LocationToMinimap(x, y);
	lines[unit] = UnitLine{ ObjectToHandle(unit), to.X, to.Y, x, y, color };
}

void MiniMapHack::delLine(void* unit)
{
	lines.erase(unit);
}

void MiniMapHack::addUnit(void* unit,unsigned int cha, unsigned int color)
{
	units[unit] = Unit{ cha, color };
}

void MiniMapHack::delUnit(void* unit)
{
	lines.erase(unit);
}

void MiniMapHack::Clear()
{
	lines.clear();
	//restore.clear();
}

void MiniMapHack::Clean()
{
	for (auto iter = lines.begin(); iter != lines.end();) {
		if (jass::IsUnitDead(iter->second.unitHandle)) {
			lines.erase(iter++);
		}
		else {
			iter++;
		}
	}
}

void MiniMapHack::Refresh()
{
	uint32_t addrW3Minimap = gameDll + 0xBE6DC4;
	uint32_t base = *(uint32_t*)(addrW3Minimap);
	if (base) {
		{
			maxX = *(int*)(base + 0x224);
			minX = *(int*)(base + 0x228);
			minY = *(int*)(base + 0x22C);
			maxY = *(int*)(base + 0x230);
			xMult = *(float*)(base + 0x21c);
			yMult = *(float*)(base + 0x220);
			unk = *(float*)(gameDll + 0x961898);
			unk2 = *(uint32_t*)(base + 0x214);
			unk3 = *(uint32_t*)(base + 0x218);

			uint32_t eax, ebx, ecx, edx;
			ebx = base;
			edx = *(uint32_t*)(ebx + 0x218);
			ecx = edx;
			//eax = *(uint32_t*)(ebx + 0x1d8);
			ecx <<= 0xA;
			//eax += ecx;
			eax = *(uint32_t*)(ebx + 0x17c);
			eax = *(uint32_t*)eax;
			eax += ecx;
			gameMiniMap = (uint32_t*)*(uint32_t*)(base + 0x1d8);

		}
	}
}

void MiniMapHack::CalMiniMapLoc(const Loc& main, Loc& mini) {
	//uint32_t W3Minimap = *(uint32_t*)(gameDll + 0xBE6DC4) + 0x750;
	//void* addrMainMapLoc2MiniMapLoc = (void *)(gameDll + 0xD0C20);
	//void* pmini = &mini;
	//void* pmain = &main;

	//_asm {
	//	push W3Minimap
	//	mov ecx, pmini
	//	mov edx, pmain
	//	call addrMainMapLoc2MiniMapLoc
	//}
	const int CAMERABOUNDFIX = 1024;

	float WorldMinX = jass::GetCameraBoundMinX().fl - CAMERABOUNDFIX;
	float WorldMaxX = jass::GetCameraBoundMaxX().fl + CAMERABOUNDFIX;
	float WorldMinY = jass::GetCameraBoundMinY().fl - CAMERABOUNDFIX;
	float WorldMaxY = jass::GetCameraBoundMaxY().fl + CAMERABOUNDFIX;
	float WorldSizeX = WorldMaxX - WorldMinX;
	float WorldSizeY = WorldMaxY - WorldMinY;
	mini.x = 255 * (main.x - WorldMinX) / WorldSizeX;
	mini.y = 255 - 255 * (main.y - WorldMinY) / WorldSizeY;
}

void MiniMapHack::DrawMiniMap()
{
	Clean();
	for (auto iter = lines.begin(); iter != lines.end(); iter++) {
		draw_line(iter->first, iter->second);
	}
	/*for (auto iter = units.begin(); iter != units.end(); iter++) {
		draw_unit(iter->first, iter->second);
	}*/
}

void MiniMapHack::draw_line(void* unit, UnitLine& obj)
{
	float ux, uy;
	ux = GetUnitX(unit), uy = GetUnitY(unit);
	if (abs(ux- obj.fx) <= 128 && abs(uy - obj.fy) <= 128) return;
	MmapLoc from = LocationToMinimap(ux, uy);
	draw_line(from.X, from.Y, obj.x, obj.y, obj.color);
}

void MiniMapHack::RestorMiniMap()
{
	for (auto it = restore.begin(); it != restore.end(); it++) {
		if (it->second == true) {
			it->second = false;
		}
		else {
			gameMiniMap[it->first] = miniMapBackup[it->first];
		}
	}
}

extern MiniMapHack* aMiniMapHack;
void MiniMapHack::DetourUpdateMiniMap()
{
	__try {
		_asm {
			push ebx
			push ecx
		}
		if (aMiniMapHack) {
			aMiniMapHack->Refresh();
			aMiniMapHack->DrawMiniMap();
		}
		_asm {
			pop ecx
			pop ebx
			call DWORD PTR DS : [miniMapcallback]
		}
	}
	__except (filter(GetExceptionCode(), GetExceptionInformation())) {
		logger->warn("MiniMapHack::DetourUpdateMiniMap crashed");
		aMiniMapHack->Clear();
	}
}
