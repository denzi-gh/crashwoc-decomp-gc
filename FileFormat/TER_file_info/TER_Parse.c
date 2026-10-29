// TER parse
//

#include "stdafx.h"
#include <Windows.h>

#pragma warning( disable: 4996 )

//Defines

//Macro
#define SFPS(pos) SetFilePointer(f, pos, 0, FILE_BEGIN)
#define SFPC(pos) SetFilePointer(f, pos, 0, FILE_CURRENT)
#define READ(v) ReadFile(f, &(v), sizeof(v), &a, NULL)
#define READP(p, n) ReadFile(f, p, n, &a, NULL)
#define WRITE(v) WriteFile(f2, &(v), sizeof(v), &a, NULL)
#define WRITEP(p, n) WriteFile(f2, p, n, &a, NULL)
#define SwapFourBytes(data)   \
( (((data) >> 24) & 0x000000FF) | (((data) >>  8) & 0x0000FF00) | \
  (((data) <<  8) & 0x00FF0000) | (((data) << 24) & 0xFF000000) ) 
#define SwapTwoBytes(data) ((((data) >> 8) & 0x00FF) | (((data) << 8) & 0xFF00))



//Structs

struct Bbox_structure
{
	//bound volume ?
	float x_max; // +14 0
	float x_min; // +18 1
	float y_max; // +1C 2
	float y_min; // +20 3
	float z_max; // +24 4
	float z_min; // +28 5

				 // some other floats probably vectors ?

	float unk_x1; // 6
	float unk_y1; // 7
	float unk_z1; // 8

	float unk_x2; // 9
	float unk_y2; // 10
	float unk_z2; // 11

	float unk_x3; // 12
	float unk_y3; // 13
	float unk_z3; // 14

	float unk_x4; // 15
	float unk_y4; // 16
	float unk_z4; // 17

				  // some floats

	float a;
	float b;
	float c;
	float d;
	float e;
	float f;

	// some flags

	unsigned char b1_flags;
	unsigned char b2_flags;
	unsigned char b3_flags;
	unsigned char pad_byte;
};

struct Float_structures
{
	WORD unk; // +0 (val >= 0 ?)
	WORD amt; // some amount +2
	float some_x_max;    // +4
	float some_x_min;	// +8
	float some_z_max;    // +C
	float some_z_min;   // +10 (this is bounds at map XZ plane?)

						//structures
	Bbox_structure* bb_structs; 
};

struct Some_Entry
{
	DWORD unk1; // 0
	DWORD shifting_amt; // 4
	float x; // 8
	float y; // C
	float z; // 10
	WORD type; // 14
	WORD unk_id; // some id of block ?
	DWORD unk2; // 18
	DWORD unk3; // 1C
	DWORD unk4; // 20
	DWORD unk5; // 24
	DWORD unk6; // 28
	DWORD unk7; // 2C
	WORD unk_w3;// +30
	WORD unk_w4;// +32
};

//Globals
Some_Entry* g_entiries = nullptr;


float ReverseFloat(const float inFloat)
{
	float retVal;
	char *floatToConvert = (char*)& inFloat;
	char *returnFloat = (char*)& retVal;

	// swap the bytes into a temporary buffer
	returnFloat[0] = floatToConvert[3];
	returnFloat[1] = floatToConvert[2];
	returnFloat[2] = floatToConvert[1];
	returnFloat[3] = floatToConvert[0];

	return retVal;
}


void Dump_float_vec(const char* vec_name, float* p_vec)
{
	if (p_vec)
	{
		printf("\t(%s) ", vec_name);
		printf("x = %f, ", ReverseFloat(*(p_vec + 0)));
		printf("y = %f, ", ReverseFloat(*(p_vec + 1)));
		printf("z = %f \n", ReverseFloat(*(p_vec + 2)));
	}
}

void Dump_bbox(float* p_bb)
{
	if (p_bb)
	{
		printf("\t(bbox) ");
		printf("x = %f, ", ReverseFloat(*(p_bb + 0)));
		printf("x_ = %f, ", ReverseFloat(*(p_bb + 1)));
		printf("y = %f, ", ReverseFloat(*(p_bb + 2)));
		printf("y_ = %f, ", ReverseFloat(*(p_bb + 3)));
		printf("z = %f,", ReverseFloat(*(p_bb + 4)));
		printf("z_ = %f \n", ReverseFloat(*(p_bb + 5)));

	}
}

void Dump_floats(HANDLE f, DWORD amount)
{
	DWORD a = 0;
	SFPS(4);

	DWORD last_f_pos = 4;
	int blk = 0;

	for (int i = 0; i < amount; i++)
	{
		printf("FP %08x \n", SFPC(0));

		printf("Entry %d\n", i);

		signed short amt = 0;

		while (amt >= 0 && (g_entiries[i].type == 1 || g_entiries[i].type == 0))
		{
			printf("\tBlock %d \n", blk++);
			printf("\tFP %08x \n", SFPC(0));

			SFPC(2); // unk word
			signed short amt;

			READ(amt);
			amt = SwapTwoBytes(amt);

			if (amt < 0)
				break;

			float some_x_max;
			float some_x_min;
			float some_z_max;
			float some_z_min;
			READ(some_x_max);
			READ(some_x_min);
			READ(some_z_max);
			READ(some_z_min);
			some_x_max = ReverseFloat(some_x_max);
			some_x_min = ReverseFloat(some_x_min);
			some_z_max = ReverseFloat(some_z_max);
			some_z_min = ReverseFloat(some_z_min);

			printf("\t Amount bb = %d | (x_max = %f, x_min = %f, z_max = %f, z_min = %f ) \n", amt,
				some_x_max, some_x_min, some_z_max, some_z_min);

			Bbox_structure* bbs = (Bbox_structure*)malloc(amt * sizeof(Bbox_structure));
			if (!bbs)
			{
				SFPC(100);
				continue;
			}
				

			for (int j = 0; j < amt; j++)
			{
				READP(bbs + j, sizeof(Bbox_structure));
				//SFPC(100);

				printf("\t--- (%d) ---\n", j);

				Dump_bbox((float*)(&(bbs[j])));

				Dump_float_vec("v1", (float*)(&(bbs[j].unk_x1)));
				Dump_float_vec("v2", (float*)(&(bbs[j].unk_x2)));
				Dump_float_vec("v3", (float*)(&(bbs[j].unk_x3)));
				Dump_float_vec("v4", (float*)(&(bbs[j].unk_x4)));

			}

			if(bbs)
				free(bbs);

		}

		if (g_entiries[i].type == 2)
		{
			SFPC(4); // useless bytes;

			printf("\tBlock type 2 \n");
			printf("\tFP %08x \n", SFPC(0));

			signed short amt;

			READ(amt);
			amt = SwapTwoBytes(amt);

			SFPC(2);

			struct vec3f {
				float x;
				float y;
				float z;
			};

			vec3f* pts = (vec3f*)malloc(amt * sizeof(vec3f));
			if (!pts)
			{
				blk = 0;
				last_f_pos += g_entiries[i].shifting_amt * 2;
				SFPS(last_f_pos); // new file pos
				continue;
			}
			
			READP(pts, amt * sizeof(vec3f));

			for (int j = 0; j < amt; j++)
			{
				printf("\t%d\n", j);
				Dump_float_vec("p", (float*)(&(pts[j])));
			}

			if(pts)
				free(pts);

		}

		blk = 0;
		last_f_pos += g_entiries[i].shifting_amt * 2;
		SFPS(last_f_pos); // new file pos
		continue;
	}
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("program [TER file]\n");
		return 1;
	}

	DWORD a = 0;
	HANDLE f = CreateFileA(argv[1], GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (!GetLastError())
	{
		DWORD offset_to_table;
		READ(offset_to_table);
		offset_to_table = SwapFourBytes(offset_to_table);
		offset_to_table *= 2;

		SFPS(offset_to_table);

		DWORD amount;
		READ(amount);
		amount = SwapFourBytes(amount);

		g_entiries = (Some_Entry*)malloc(sizeof(Some_Entry)*amount);

		SFPC(-4);

		int counter = 0;
		int f_ptr = 4;

		for (int i = 0; i < amount; i++)
		{
			READP(&(g_entiries[i]), sizeof(Some_Entry));
			g_entiries[i].shifting_amt = SwapFourBytes(g_entiries[i].shifting_amt);
			g_entiries[i].x = ReverseFloat(g_entiries[i].x);
			g_entiries[i].y = ReverseFloat(g_entiries[i].y);
			g_entiries[i].z = ReverseFloat(g_entiries[i].z);
			g_entiries[i].type = SwapTwoBytes(g_entiries[i].type);
			g_entiries[i].unk_id = SwapTwoBytes(g_entiries[i].unk_id);


			if (g_entiries[i].type == 2)
			{
				f_ptr += g_entiries[i].shifting_amt * 2 + 4; // skipping 12 34 56 78 bytes
			}
			else
			{
				f_ptr += g_entiries[i].shifting_amt * 2;
			}
			

			printf("Entry %d f_ptr = %08x (type = %d, id = %d)\n", i, f_ptr, g_entiries[i].type, g_entiries[i].unk_id);

		}
		printf("f_ptr = %08x\n", f_ptr);

		Dump_floats(f, amount);

	}
	else
	{
		printf("Error open file (%08x)\n", GetLastError());
	}

	//system("PAUSE");
	return 0;
}

