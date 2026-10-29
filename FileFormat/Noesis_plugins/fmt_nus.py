#by Durik256
from inc_noesis import *

def registerNoesisTypes():
    handle = noesis.register("Crash", ".nus")
    noesis.setHandlerTypeCheck(handle, CheckType)
    noesis.setHandlerLoadModel(handle, LoadModel)
    return 1

def CheckType(data):
    if data[:4] != b'0CSG':
        return 0
    return 1

def LoadModel(data, mdlList):
    global names, txList, matList, mshOfs, matrix, bones
    names, txList, matList, mshOfs, matrix, bones = None, [], [], 0, {}, []
    
    bs = NoeBitStream(data, 1)
    ctx = rapi.rpgCreateContext()
    
    rapi.rpgSetEndian(1)
    
    readChunks(bs, bs.getSize())
    
    #create mesh
    print('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>',mshOfs)
    bs.seek(mshOfs)
    gobjCount = bs.readUInt()
    print("gobjCount: ", gobjCount)
            
    for x in range(gobjCount):
        nus_geom(bs, x)

    #rapi.rpgOptimize()
    mdl = rapi.rpgConstructModel()#NoeModel()#
    mdl.setModelMaterials(NoeModelMaterials(txList, matList))
    mdl.setBones(bones)
    mdlList.append(mdl)
    rapi.setPreviewOption("setAngOfs", "0 90 90")
    return 1
    
def readChunks(bs, eof):
    while bs.tell() < eof:
        curPos = bs.tell()
        id, size = bs.read(4), bs.readInt()
        child = False
        print(id, size, curPos)
        
        if id == b'0CSG':
            child = True
        
        elif id == b'LBTN':
            global names
            names = bs.read(bs.readInt())
        
        elif id == b'0TST':
            child = True
        
        elif id == b'0HST':
            countTx = bs.readInt()
            print('countTx::',countTx)
        
        elif id == b'0MXT':
            global txList
            type = bs.readInt()
            width = bs.readInt()
            height = bs.readInt()
            data = bs.read(bs.readInt())
            
            try:
                if type == 128:
                    data = untileDXT(data, width, height)
                elif type == 129:
                    data = untileRGBA(data, width, height)
                else:
                    data = untileRGBA(data, width, height)#This is incorrect
            except:
                data = b'\x00'*(width*height*4)
            
            data = rapi.imageDecodeRaw(data, width, height, 'b8g8r8a8')
            
            print('texture:: type:',type,'size:',[width,height])
            name = 'tx_%i'%len(txList)
            txList.append(NoeTexture(name, width, height, data, noesis.NOESISTEX_RGBA32))
        
        elif id == b'00SM':
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
        
        elif id == b'0TSG':
            '''
            gobjCount = bs.readUInt()
            print("gobjCount: ", gobjCount)
            
            for x in range(gobjCount):
                nus_geom(bs)
            
            break
            '''
            global mshOfs
            mshOfs = bs.tell()
        
        elif id == b'TSNI':
            matrixCount = bs.readUInt()
            print("matrixCount: ", matrixCount)
            
            global matrix
            for x in range(matrixCount):
                InvInitMatrix = NoeMat44.fromBytes(bs.read(64),1).toMat43()
                ModelIndex, UnknownUInt32_1, UnknownUInt32_2, zero = bs.read('>4I')
                print(ModelIndex, UnknownUInt32_1, UnknownUInt32_2)

                matrix[ModelIndex] = InvInitMatrix#.swapHandedness(2)
                #matrix.append(InvInitMatrix)
            #break
        elif id == b'0TSS':
            entryCount = bs.readInt()
            bs.seek(4,1)
            
            global bones

            for x in range(entryCount):
                pointCount = bs.readInt()
                nameOffsets = bs.readInt()

                for x in range(pointCount):
                    pos = NoeVec3.fromBytes(bs.read(12),1)
                    mat = NoeMat43()
                    mat[3] = pos
                    bones.append(NoeBone(x,'spline_%i'%x,mat,None,x-1))
            
        else:
            pass
        
        if curPos+size > eof: size = eof-curPos
        
        if child:
            readChunks(bs, curPos+size)
    
        bs.seek(curPos+size)#curPos + size)
    
def new_geom(bs):
    UnknownCount1 = bs.readUInt()
    for x in range(UnknownCount1):

        IsFace = bs.readUInt()
        if (IsFace == 0):
            UnknownData = bs.readBytes(0xC)
            geometryDefinitionCount = bs.readUInt()
            
            for x in range(geometryDefinitionCount):
                MaterialIndex = bs.readUInt()
                
                #ReadVertices
                vertexCount = bs.readUInt()
                for x in range(vertexCount):
                    bs.seek(36,1)
                
                #ReadCntrl
                n = bs.readUInt()
                        
                #ReadPrimitives
                primitiveCount = bs.readUInt()

                for x in range(primitiveCount):
                    Type = bs.readUInt()
                    length = bs.readUInt()
                    for x in range(length):
                        bs.readUShort()
                
                #ReadSkins
                skinCount = bs.readUInt()
                for x in range(skinCount):
                    print("Geometry skin not yet implemented.")
                    return 0
                
                #ReadBlendShapes
                blendShapeNumber = bs.readUInt()
                if (blendShapeNumber > 0):
                    pass
                    #sorcery

        elif (IsFace == 1):
            faceCount = bs.readUInt()
            for x in range(faceCount):
                a = bs.readUInt()
                Count = bs.readUInt()
                b = bs.readUInt()
                c = bs.readUInt()

                Data = bs.readBytes(Count * 0x18)
    
def nus_geom(bs, matrix_id=-1):
    models_n = bs.readInt()
    print('models_n:',models_n)
    
    global matrix
    rapi.rpgSetTransform(None)
    
    #if matrix_id != -1:
    #    rapi.rpgSetTransform(matrix[matrix_id])
    
    try:
        rapi.rpgSetTransform(matrix[matrix_id])
    except:
        pass

    for x in range(models_n):
        type = bs.readInt()
        print('nus_geom::type:',type)

        if type == 0:
            bs.seek(12,1)
            
            meshes_n = bs.readInt()
            print('nus_geom::meshes_n:',meshes_n)
            for x in range(meshes_n):
                material_id = bs.readInt()
                print('nus_geom::material_id:',material_id)

                vertices_n = bs.readInt()
                print('nus_geom::vertices_n:',vertices_n)
                
                rapi.rpgSetMaterial('mat_%i'%material_id)

                vbuf = bs.read(vertices_n*36)
                rapi.rpgBindPositionBuffer(vbuf, noesis.RPGEODATA_FLOAT, 36)
                rapi.rpgBindNormalBufferOfs(vbuf, noesis.RPGEODATA_FLOAT, 36, 12)
                #rapi.rpgBindColorBufferOfs(vbuf, noesis.RPGEODATA_UBYTE, 36, 24, 4)
                rapi.rpgBindUV1BufferOfs(vbuf, noesis.RPGEODATA_FLOAT, 36, 28)
                
                cbuf = b''
                cs = NoeBitStream(vbuf)
                for c in range(vertices_n):
                    cs.seek(24,1)
                    #cbuf += cs.read(4)[::-1]
                    c0=cs.read(1)
                    c1=cs.read(1)
                    c2=cs.read(1)
                    c3=cs.read(1)
                    cbuf += c1#b
                    cbuf += c2#g
                    cbuf += c3#r
                    cbuf += c0#a
                    cs.seek(8,1)
                rapi.rpgBindColorBuffer(cbuf, noesis.RPGEODATA_UBYTE, 4, 4)
                
                primOfs = bs.tell()
                geom_cntrl = bs.readInt() #Only 0.
                primitives_n = bs.readInt()
                for x in range(primitives_n):
                    primtype = bs.readInt()
                    indices_n = bs.readInt()

                    if primtype == 6:
                        print('================================')
                        cur = bs.tell()
                        while bs.tell() < (cur + indices_n*2):
                            num = bs.readShort()
                            indices = bs.read(num*2)
                            rapi.rpgCommitTriangles(indices, noesis.RPGEODATA_USHORT, num, noesis.RPGEO_TRIANGLE_STRIP)
                    if primtype == 5:
                        indices = bs.read(indices_n*2)
                        rapi.rpgCommitTriangles(indices, noesis.RPGEODATA_USHORT, indices_n, noesis.RPGEO_TRIANGLE)
                rapi.rpgClearBufferBinds()
                
                geomskin_present = bs.readInt() 

                if geomskin_present != 0:
                    type = bs.readByte()

                    if type == 0:
                        unknown = bs.readInt()
                        unknown_len = bs.readInt()
                        unknown_n = bs.readInt()
                        bs.seek(unknown_n*4,1)
                        bs.seek(unknown_len*unknown_n*4,1)
                    else:
                        bs.seek(vertices_n*16,1)
                
                blendshapes_n = bs.readInt()
                print('blendshapes_n::',blendshapes_n)
                if blendshapes_n != 0:
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
                bs.seek(0x18*unknown1_n,1)
                

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