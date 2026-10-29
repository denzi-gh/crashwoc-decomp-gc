//Some file that can be collision ?
//.ter
//
//big-endian
/*
               TER file
            --------------
          ╔═|  Offset/2  | 4b
          ║ |------------|
          ║ | Float_Data | nb
          ║ |------------|
          ╠►|   Amount   | 4b
          ║ |------------|
          ╚►|   Entries  | Amount * 52b (also they merged with amount field)
            --------------
        	
	Float_Data:
        Has different types of structure:
        Type 0 and 1 has lenght 20b + amt * 100b
        Type 2 has lenght 12b * amount_points + 4b
        
        Every entry points to file location inside Float_Data: f_ptr
        Structures
        Type 0 and Type 1:
        Inside is bounding volume and points (triangle or quad)
	
	    ------------------------
f_ptr-> |   type_0_and_1_hdr   | 20b
	    |----------------------|
	    | Triangles structures | amt * 100b (amt from Float_structures_type_0_and_1_hdr)
	    |----------------------|
	    |   type_0_and_1_hdr   | repeats...
	    |----------------------|
	    | Triangles structures |
	    |----------------------|
	    |		   ...         |
	    |----------------------|
	    |       FFFFFFFF       | // stop sequence (detected by (amt < 0))
	    |----------------------|
	
		size of structure is:
		4 + 16 + 100*amt bytes
		if amt < 0 then loop stops, and next f_ptr will chosen by
		adding offset to f_ptr
		
		f_ptr += f_ptr + g_Entries[curr_entry].shifting_amt * 2;
		
		(b3_flags) is display some information.
		02 = inside (in cave, in arc, etc...)
		03 = outside
		others... idk...
		
		This structure need to be shifted with {x, y, z} inside Table_Entry
		
		Type 2: 
		Probably wall collision.
		Encountered with following bytes:
		12 34 56 78
		
		|----------------------|
		|      12 34 56 78     | 4b (signature?)
	    |----------------------|		
f_ptr-> |    amount_points     | 2b		
	    |----------------------|		
	    |          unk         | 2b		
	    |----------------------|		
	    |      Vec3f points    | 12b * amount_points	
	    |----------------------|		
		
		Looks like a mesh. Y component of points not make sense.
		Probably collision ignores Y component for walls anyway.
		
		Points make 2D mesh with TRIANGLE_STRIP method.
		
		This structure not need to be shifted with {x, y, z} inside Table_Entry
		
		Type 3:
		???
		Can be just for testing
	
	Entries:
		see Table_Entry struct;
		shifting_amt is size of Float_Data memory block that
		belong to this specific entry.
	
*/
// amount max is 2047
// f_ptr = 4 from start;

// Type 0 - shifted with {x, y, z} inside Table_Entry
// Type 1 - ... same ?
// Type 2 - Like mesh structure... wall collisions ?
// Type 3 - testing ?

// This for Type 0 and Type 1

struct Vec3f {
	float x;
	float y;
	float z;
}

struct Triangles_structure
{
	//bounding volume
	float x_max; // +14 0
	float x_min; // +18 1
	float y_max; // +1C 2
	float y_min; // +20 3
	float z_max; // +24 4
	float z_min; // +28 5
	
	// points   //     p3----p4
	Vec3f p1;   //	   | \   |
	Vec3f p2;   //     |  \  |
	Vec3f p3;   //     |   \ |
	Vec3f p4;   //     |    \|
                //	   p1----p2
	            // 
	// if {x,y,z} of p4 is all zeros, whole structure
	// describing triangle, instead this is quad.
	
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
	unsigned char b3_flags; // 0x02 inside, 0x03 outside.
	unsigned char pad_byte; 
} // size 100b

struct Float_structures_type_0_and_1_hdr
{
	WORD unk; // +0 (val >= 0 ?)
	WORD amt; // some amount +2
	float some_x_max;    // +4
	float some_x_min;	// +8
	float some_z_max;    // +C
	float some_z_min;   // +10 (this is bounds at map XZ plane?)
} // size 20b

struct Float_structures_type_2
{
	WORD amt; // amount ?
	WORD unk;
	Vec3f points[amt];
} // size 4b + 12b * amt

struct Table_Entry
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
}; // Size 52 b



// this one used in game not in file...
struct Some_terrain_struct
{
	float x; //0  this is shifting vector {x,y,z} every
	float y; //4  triangle and quad points that belong to this
	float z; //8  struct need to be shifted to position properly
	         //   except type 2 structure.
	
	void* p_data; //C points at file data (f_ptr)
	
	float bb_x_max;  //10 
	float bb_y_max; //14
	float bb_z_max; //18
	float bb_x_min; //1c
	float bb_y_min; //20
	float bb_z_min; //24 // bounding volume 
	
	DWORD unk6; //28 unk7 from above
	DWORD type; //2c Type from above (init value -1)
	WORD unk_w1; //30 unk_w1 from above
	WORD unk_w2; //32 unk_w3 from above
	float minimal_lenght; //34 min(len(point_vectors)^2) ???
	DWORD size_of_this_struct;//38 ??? idk where this value came from.
	DWORD unk11;//3c
}