#by Durik256
from inc_noesis import *

def registerNoesisTypes():
    handle = noesis.register("Crash", ".hgo")
    noesis.setHandlerTypeCheck(handle, CheckType)
    noesis.setHandlerLoadModel(handle, LoadModel)
    noesis.setHandlerWriteModel(handle, noepyWriteModel)
    return 1

def CheckType(data):
    if data[:4] != b'FOGH':
        return 0
    return 1

def LoadModel(data, mdlList):
    global names, bones, txList, matList
    names, bones, txList, matList = None, [], [], []
    
    bs = NoeBitStream(data, 1)
    ctx = rapi.rpgCreateContext()
    
    rapi.rpgSetEndian(1)
    
    readChunks(bs, bs.getSize())

    rapi.rpgOptimize()
    mdl = rapi.rpgConstructModel()#NoeModel()
    mdl.setBones(bones)
    mdl.setModelMaterials(NoeModelMaterials(txList, matList))#[NoeMaterial('default','')]
    mdlList.append(mdl)
    rapi.setPreviewOption("setAngOfs", "0 90 90")
    return 1
    
def readChunks(bs, eof):
    while bs.tell() < eof:
        curPos = bs.tell()
        id, size = bs.read(4), bs.readInt()
        child = False
        print(id, size, curPos)
        
        if id == b'FOGH':
            child = True
        
        if id == b'LBTN':
            global names
            names = bs.read(bs.readInt())
            #[x.decode() for x in bs.read(bs.readInt()).split(b'\x00')]
        
        if id == b'0TST':
            child = True
        
        if id == b'0HST':
            countTx = bs.readInt()
            print('countTx::',countTx)
        
        if id == b'0MXT':
            global txList#, matList
            type = bs.readInt()
            width = bs.readInt()
            height = bs.readInt()
            data = bs.read(bs.readInt())
            
            if type == 128:
                data = untileDXT(data, width, height)
            elif type == 129:
                data = untileRGBA(data, width, height)
            else:
                data = untileRGBA(data, width, height)#This is incorrect
            
            data = rapi.imageDecodeRaw(data, width, height, 'b8g8r8a8')
            
            print('texture:: type:',type,'size:',[width,height])
            name = 'tx_%i'%len(txList)
            txList.append(NoeTexture(name, width, height, data, noesis.NOESISTEX_RGBA32))
            #matList.append(NoeMaterial(name,name))
        
        if id == b'00SM':
            global matList
            
            materials_n = bs.readInt()
            for x in range(materials_n):
                m_inf = bs.read('>21i')
                
                r = struct.unpack('f', struct.pack('i', m_inf[5]))[0]
                g = struct.unpack('f', struct.pack('i', m_inf[6]))[0]
                b = struct.unpack('f', struct.pack('i', m_inf[7]))[0]

                bitmap_id = m_inf[14]
                nameTx = ''
                if bitmap_id >= 0:
                    print('bitmap_id::',bitmap_id)
                    nameTx = txList[bitmap_id].name
                
                localMat = NoeMaterial('mat_%i'%x, nameTx)
                localMat.setDiffuseColor(NoeVec4([r,g,b,1]))
                matList.append(localMat)
        
        if id == b'0OGH':
            global bones
            bones_n = bs.readUByte()
            for x in range(bones_n):
                unk = bs.readUByte()
                mat = NoeMat44.fromBytes(bs.read(64),1).toMat43().inverse()#rot 
                mat[3] = NoeMat44.fromBytes(bs.read(64),1).toMat43().inverse()[3]#pos 
                scl = NoeMat44.fromBytes(bs.read(64),1)
                unk0 = bs.readInt()
                unk1 = bs.readFloat()
                unk2 = bs.readFloat()
                parent_id = bs.readByte()
                name_offset = bs.readInt()-1
                name = 'bone_%i'%x
                try:
                    print('bone_name_offset:',name_offset)
                    name = names[name_offset:].split(b'\x00')[0].decode()
                except: 
                    print('error_name_offset::',name_offset)
                bones.append(NoeBone(x,name,mat,None,parent_id))
            
            print('xxxxxxxxxxxxxxxxxxxxxxxxxx', bs.tell() )
            unknown0_len = bs.readByte()
            bs.seek(unknown0_len,1)#unknown0
            unknown1_len = bs.readByte()
            bs.seek(unknown1_len,1)#unknown1
            layers_n = bs.readByte()
            print('xxxxxxxxxxxxxxxxxxxxxxxxxx', unknown0_len,unknown1_len,layers_n, bs.tell() )
            for x in range(layers_n):
                name_offset = bs.readInt()-1
                primary_bone_meshes_present = bs.readUByte()
                print('primary_bone_meshes_present::',primary_bone_meshes_present)
                if primary_bone_meshes_present != 0:
                    for x in range(bones_n):
                        primary_bone_mesh_present = bs.readUByte()
                        if primary_bone_mesh_present != 0:
                            hgo_geom(bs,x)
                
                
                primary_mesh_present = bs.readUByte()
                print('primary_mesh_present::',primary_mesh_present)
                if primary_mesh_present != 0:
                    hgo_geom(bs)
                
                secondary_bone_meshes_present = bs.readUByte()
                print('secondary_bone_meshes_present::',secondary_bone_meshes_present)
                if secondary_bone_meshes_present != 0:
                     for x in range(bones_n):
                        secondary_bone_mesh_present = bs.readUByte()
                        if secondary_bone_mesh_present != 0:
                            hgo_geom(bs)
                
                secondary_mesh_present = bs.readUByte()
                print('secondary_mesh_present::',secondary_mesh_present)
                if secondary_mesh_present != 0:
                    hgo_geom(bs)
                    
            point_count = bs.readUByte()
            for x in range(point_count):
                position = NoeMat44.fromBytes(bs.read(64),1).toMat43().inverse()
                name_offset = bs.readInt()-1
            
            #unk....
            
            print(bs.tell())
            break
        else:
            pass
        
        if curPos+size > eof: size = eof-curPos
        
        if child:
            readChunks(bs, curPos+size)
    
        bs.seek(curPos+size)#curPos + size)
    
def hgo_geom(bs, boneID=-1):
    models_n = bs.readInt()
    print('models_n:',models_n)

    for x in range(models_n):# Only occurs as 1.
        type = bs.readInt()
        print('hgo_geom::type:',type)
        #print('hgo_geom::type:',type)
        if type == 0:
            unknown0 = bs.readInt()
            unknown1 = bs.readInt()
            unknown2 = bs.readInt()
            meshes_n = bs.readInt() #The model is split into meshes according to its materials.
            print('hgo_geom::meshes_n:',meshes_n)
            for x in range(meshes_n):
                material_id = bs.readInt()
                print('hgo_geom::material_id:',material_id)
                #print('material_id::',material_id)
                vertices_n = bs.readInt()
                print('hgo_geom::vertices_n:',vertices_n)
                
                #global matList
                rapi.rpgSetMaterial('mat_%i'%material_id)#'mat_%i'%material_id
                #print('hgo_geom::vertices_n:',vertices_n)
                vbuf = bs.read(vertices_n*36)
                rapi.rpgBindPositionBuffer(vbuf, noesis.RPGEODATA_FLOAT, 36)
                rapi.rpgBindNormalBufferOfs(vbuf, noesis.RPGEODATA_FLOAT, 36, 12)
                #rapi.rpgBindColorBufferOfs(vbuf, noesis.RPGEODATA_FLOAT, 24, 4, 4)
                rapi.rpgBindUV1BufferOfs(vbuf, noesis.RPGEODATA_FLOAT, 36, 28)
                
                '''
                for x in range(vertices_n):
                    position = NoeVec3.fromBytes(bs.read(12),1)
                    normals = NoeVec3.fromBytes(bs.read(12),1)
                    colour = bs.readInt() # ARGB format.
                    uv = NoeVec3([bs.readFloat(),bs.readFloat(),0])
                '''
                
                primOfs = bs.tell()
                geom_cntrl = bs.readInt() #Only 0.
                primitives_n = bs.readInt()
                for x in range(primitives_n):
                    primtype = bs.readInt()
                    indices_n = bs.readInt()
                    bs.seek(indices_n*2,1)
                
                geomskin_present = bs.readInt() # Yes, this is a 32-bit boolean.
                #print('geomskin_present::',geomskin_present, bs.tell())
                if geomskin_present != 0: #; This is known in Blender lingo as bone weights.
                    type = bs.readByte()
                    #print('>>>>>>type::',type)
                    if type == 0: # This type is unused.
                        unknown = bs.readInt()
                        unknown_len = bs.readInt()
                        unknown_n = bs.readInt()
                        bs.seek(unknown_n*4,1)#i32 unknown_a [unknown_n]
                        bs.seek(unknown_len*unknown_n*4,1)#i32 unknown_b [unknown_len] [unknown_n]
                    else:
                        '''
                        for x in range(vertices_n):
                            influence0 = bs.readFloat()
                            influence1 = bs.readFloat()
                            influence2 = bs.readFloat()
                            bone_id0 = bs.readByte()
                            bone_id1 = bs.readByte()
                            bone_id2 = bs.readByte()
                            unused = bs.readByte()
                         '''
                        wbuf = bs.read(vertices_n*16)
                        rapi.rpgBindBoneWeightBuffer(wbuf, noesis.RPGEODATA_FLOAT, 16, 3)
                        rapi.rpgBindBoneIndexBufferOfs(wbuf, noesis.RPGEODATA_BYTE, 16, 12, 3)
                        
                        rapi.rpgSetTransform(None)
                else:
                    if boneID != -1:
                        wbuf = noePack('BB',255,boneID)*vertices_n
                        rapi.rpgBindBoneWeightBuffer(wbuf, noesis.RPGEODATA_BYTE, 2, 1)
                        rapi.rpgBindBoneIndexBufferOfs(wbuf, noesis.RPGEODATA_BYTE, 2, 1, 1)
                
                        rapi.rpgSetTransform(bones[boneID].getMatrix())
                read_Prim(bs, primOfs)
                
                blendshapes_n = bs.readInt()
                if blendshapes_n != 0: # Known in Blender lingo as shape keys.
                    blendshapes_ids = bs.read('%iI'%blendshapes_n)
                    blendshape_len= bs.readInt()
                    blendshape = bs.read('%ib'%blendshape_len)
                    blendshape_presence = bs.read('%ib'%blendshapes_n)

        elif type == 1: # Unused.
            unknown_n = bs.readInt()
            for x in range(unknown_n):
                unknown0 = bs.readInt()
                unknown1_n = bs.readInt()
                unknown2 = bs.readInt()
                unknown3 = bs.readFloat()
                bs.seek(0x18*unknown1_n,1)#unknown1 [0x18] [unknown1_n]
     
def read_Prim(bs, primOfs):
    curPos = bs.tell() 
    bs.seek(primOfs)
    geom_cntrl = bs.readInt() #Only 0.
    primitives_n = bs.readInt()
    for x in range(primitives_n):
        primtype = bs.readInt()
        indices_n = bs.readInt()
        #indices = bs.read(indices_n*2)

        if primtype == 6:
            cur = bs.tell()
            while bs.tell() < (cur + indices_n*2):
                num = bs.readShort()
                indices = bs.read(num*2)
                rapi.rpgCommitTriangles(indices, noesis.RPGEODATA_USHORT, num, noesis.RPGEO_TRIANGLE_STRIP)
        if primtype == 5:
            indices = bs.read(indices_n*2)
            rapi.rpgCommitTriangles(indices, noesis.RPGEODATA_USHORT, indices_n, noesis.RPGEO_TRIANGLE)
    bs.seek(curPos)

#-------------------------TEXTURE-----------------------
def untileDXT(PixelData, Width, Height):
    bs = NoeBitStream(PixelData,1)
    image = []
    for y in range(Height):
        xi = []
        for x in range(Width):
            xi.append(0)
        image.append(xi)
    
    index = 0
    for y in range(0,Height,8):
        for x in range(0,Width,8):
            DecodeDXTBlock(image, bs, index, x, y)
            index += 8
            DecodeDXTBlock(image, bs, index, x + 4, y)
            index += 8
            DecodeDXTBlock(image, bs, index, x, y + 4)
            index += 8
            DecodeDXTBlock(image, bs, index, x + 4, y + 4)
            index += 8
            
    data = b''
    for y in range(Height):
        for x in range(Width):
            data += image[y][x].to_bytes(4, 'little')
        
    return data

def DecodeDXTBlock(image, src, srcOffset, blockX, blockY):
    if srcOffset >= src.getSize():
        return 0
    src.seek(srcOffset)
    c1 = src.readUShort()
    src.seek(srcOffset + 2)
    c2 = src.readUShort()
    lines = src.read('4B');
    blue1 = Convert5To8((c1 & 0x1F))
    blue2 = Convert5To8((c2 & 0x1F));
    green1 = Convert6To8(((c1 >> 5) & 0x3F))
    green2 = Convert6To8(((c2 >> 5) & 0x3F))
    red1 = Convert5To8(((c1 >> 11) & 0x1F))
    red2 = Convert5To8(((c2 >> 11) & 0x1F))

    colors = [0,0,0,0]
    colors[0] = IntfromRGBA(red1, green1, blue1, 255)
    colors[1] = IntfromRGBA(red2, green2, blue2, 255)
    if (c1 > c2):
        colors[2] = IntfromRGBA(DXTBlend(red2, red1), DXTBlend(green2, green1), DXTBlend(blue2, blue1), 255)
        colors[3] = IntfromRGBA(DXTBlend(red1, red2), DXTBlend(green1, green2), DXTBlend(blue1, blue2), 255)
    else:
        colors[2] = IntfromRGBA(((red1 + red2) // 2), ((green1 + green2) // 2), ((blue1 + blue2) // 2), 255)
        colors[3] = IntfromRGBA(((red1 + red2) // 2), ((green1 + green2) // 2), ((blue1 + blue2) // 2), 0)

    for y in range(4):
        val = lines[y]
        for x in range(4):
            image[y + blockY][x + blockX] = colors[(val >> 6) & 3]
            val <<= 2

def Convert5To8(v):
    return ((v << 3) | (v >> 2))
    
def Convert6To8(v):
    return ((v << 2) | (v >> 4))
    
def Convert3To8(v):
    return ((v << 5) | (v << 2) | (v >> 1))
 
def Convert4To8(v):
    return ((v << 4) | v)
 
def DXTBlend(v1, v2):
    return ((v1 * 3 + v2 * 5) >> 3)
    
def IntfromRGBA(red, green, blue, alpha):
    RGBint = (alpha << 24) + (red << 16) + (green << 8) + blue
    return RGBint

def untileRGBA(PixelData, Width, Height):
    bs = NoeBitStream(PixelData,1)
    image = []
    for y in range(Height):
        xi = []
        for x in range(Width):
            xi.append(0)
        image.append(xi)
    
    index = 0
    for y in range(0,Height,4):
        for x in range(0,Width,4):
            for by in range(4):
                for bx in range(4):
                    DecodeRGB5A3Block(image, bs, index, x + bx, y + by)
                    index += 2
                
    data = b''
    for y in range(Height):
        for x in range(Width):
            data += image[y][x].to_bytes(4, 'little')
        
    return data
    
def DecodeRGB5A3Block(dst, src, srcOffset, x, y):
    if srcOffset >= src.getSize():
        return 0
    c = src.readUShort()


    if ((c & 0x8000) != 0):
        r = Convert5To8((c >> 10) & 0x1F)
        g = Convert5To8((c >> 5) & 0x1F)
        b = Convert5To8(c & 0x1F)
        a = 0xFF
    else:
        a = Convert3To8((c >> 12) & 0x7)
        b = Convert4To8((c >> 8) & 0xF)
        g = Convert4To8((c >> 4) & 0xF)
        r = Convert4To8(c & 0xF)

    dst[y][x] = IntfromRGBA(r, g, b, a)
    
#-------------------------EXPORT-----------------------
def noepyWriteModel(mdl, bs):
    pass