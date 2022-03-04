using System;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using System.Text;

#if UNITY_EDITOR_WIN

using UnityEditor;

#endif

/* TODO - When writing data back to file, work out the math for updating ground textures without removing sound filenames */

public class WLDFileLoader : MonoBehaviour {

    private MeshFilter mfGroundInfo = null;
    private MeshRenderer mrGroundGen = null;
    private int nByteFileReadPos = 0;
    private Dictionary<string, Dictionary<string, Vector3[]>> dictModelCoords = new Dictionary<string, Dictionary<string, Vector3[]>>();
    private Dictionary<string, List<string>> dictModelNamesByType = new Dictionary<string, List<string>>();
    private List<Material> ltmtlGroundTxtrs = new List<Material>();
    private List<MaterialPosInfo> ltmtlTxtPosInfo = new List<MaterialPosInfo>();
    private List<Material> ltmtlGroundMapList = new List<Material>();
    private string strFileSelPath = "";
    private long lHeightMapLen = 0;
    private long lTxtrDataLen = 0;
    private int nMapSize = 0;
    private int nSideLen = 0;
    private byte[] abyFileData = null;
    private byte[] abyHeightMap = null;
    private byte[] abyTxtrData = null;
    private Dictionary<string, string> dictGroundSounds = new Dictionary<string, string>();
    private string strWaterFileName = "";

    // Start is called before the first frame update
    void Start() {

        Setup();
    }

    private void Setup() {

        if (mrGroundGen == null) {

            mrGroundGen = gameObject.AddComponent<MeshRenderer>();
        }

        if (mfGroundInfo == null) {

            mfGroundInfo = gameObject.AddComponent<MeshFilter>();
        }
    }

    public void GenerateGround(string strFilePath) {

        FileStream fsAccess = null;
        byte[] abyFileSectData;
        int nFileCount = 0;
        string strFoundText = "";
        string strGroundTxtrFileNames = "";
        List<string> ltstrBuildings;
        List<string> ltstrShapes;
        List<string> ltstrTrees;
        List<string> ltstrGrass;
        List<string> ltstrAnimPrime;
        List<string> ltstrAnimSec;
        List<string> ltstrDungeons;
        int nTxtrIndex = -1;
        int nTxtrCheckIndex = -1;
        int nTxtrDataIndex = -1;
        int nTxtrDataLastIndex = 0;
        int nStartIndex = 0;
        int nMatPosIndex = -1;
        List<Vector3> ltv3Verts = new List<Vector3>();
        List<Vector3> ltv3Nrmls = new List<Vector3>();
        List<Vector3> ltv3SubmitVerts;
        List<Vector3> ltv3SubmitNrmls;
        List<CombineInstance> ltciMeshes = new List<CombineInstance>();

        try {

            Debug.Log("Loading file '" + strFilePath + "' started.");

            strFileSelPath = strFilePath;

            Setup();

            if (dictModelNamesByType.Count > 0) {

                mfGroundInfo.sharedMesh.Clear();
                GameObject.Destroy(mfGroundInfo.sharedMesh);

                foreach (Dictionary<string, Vector3[]> dictModelList in dictModelCoords.Values) {

                    foreach (string strModelName in dictModelList.Keys) {

                        RemoveModelIndicatorsByName(strModelName);
                    }
                }

                dictModelCoords.Clear();
                dictModelNamesByType.Clear();
                ltmtlTxtPosInfo.Clear();
                ltmtlGroundTxtrs.Clear();
                ltmtlGroundMapList.Clear();
                dictGroundSounds.Clear();
            }

            fsAccess = new FileStream(strFilePath, FileMode.Open, FileAccess.Read);
            abyFileData = new byte[(int)fsAccess.Length];
            fsAccess.Read(abyFileData, 0, (int)fsAccess.Length);
            fsAccess.Close();

            nMapSize = (Convert.ToInt32(abyFileData[5]) << 8) | Convert.ToInt32(abyFileData[6]);
            nSideLen = (nMapSize / 2) + 1;
            lHeightMapLen = ((((nMapSize / 2) + 1) * ((nMapSize / 2) + 1)) * 2);
            lTxtrDataLen = (((nMapSize / 2) + 1) * ((nMapSize / 2) + 1));

            abyHeightMap = new byte[lHeightMapLen];
            abyTxtrData = new byte[lTxtrDataLen];

            Buffer.BlockCopy(abyFileData, 8, abyHeightMap, 0, (int)lHeightMapLen);
            Buffer.BlockCopy(abyFileData, (int)lHeightMapLen + 8, abyTxtrData, 0, (int)lTxtrDataLen);

            nByteFileReadPos = (int)(lHeightMapLen + 8 + lTxtrDataLen);
            nFileCount = BitConverter.ToInt32(abyFileData, nByteFileReadPos);

            nByteFileReadPos += 4;

            abyFileSectData = new byte[256];

            for (int nCounter = 0; nCounter < nFileCount; nCounter++) {

                Buffer.BlockCopy(abyFileData, nByteFileReadPos, abyFileSectData, 0, 256);

                if (strGroundTxtrFileNames != "") {

                    strGroundTxtrFileNames += ", ";
                }

                strFoundText = Encoding.UTF8.GetString(abyFileSectData);
                strFoundText = strFoundText.Substring(0, strFoundText.IndexOf(".") + 4);
                strGroundTxtrFileNames += strFoundText;
                strFoundText = strFoundText.Replace(".tga", ".dds");
                
                ltmtlGroundTxtrs.Add(CreateMaterial(strFoundText));

                nByteFileReadPos += 260;
                Buffer.BlockCopy(abyFileData, nByteFileReadPos, abyFileSectData, 0, 256);

                if (Encoding.UTF8.GetString(abyFileSectData).IndexOf(".wav") > 0) {

                    dictGroundSounds.Add(strFoundText,
                                         Encoding.UTF8.GetString(abyFileSectData).Substring(0, Encoding.UTF8.GetString(abyFileSectData).IndexOf(".") + 4));
                }

                nByteFileReadPos += 256;
            }

            Buffer.BlockCopy(abyFileData, nByteFileReadPos, abyFileSectData, 0, 256);
            strWaterFileName = Encoding.UTF8.GetString(abyFileSectData).Substring(0, Encoding.UTF8.GetString(abyFileSectData).IndexOf(".") + 4);
            nByteFileReadPos += 256;

            Debug.Log("Map Size: " + nMapSize);
            Debug.Log("Ground Textures: " + strGroundTxtrFileNames);

            ltstrBuildings = GetModelFileList("smod");
            dictModelCoords.Add("Buildings", CreateModelIndicators(ReadModelFileData(), ltstrBuildings, "BLG"));

            ltstrShapes = GetModelFileList("smod");
            dictModelCoords.Add("Shapes", CreateModelIndicators(ReadModelFileData(), ltstrShapes, "SHAPE"));

            ltstrTrees = GetModelFileList("smod");
            dictModelCoords.Add("Trees", CreateModelIndicators(ReadModelFileData(), ltstrTrees, "TREE"));

            ltstrGrass = GetModelFileList("smod");
            dictModelCoords.Add("Grass", CreateModelIndicators(ReadModelFileData(), ltstrGrass, "GRASS"));

            ltstrAnimPrime = GetModelFileList("vani");
            dictModelCoords.Add("Animations - Primary", CreateModelIndicators(ReadModelFileData(), ltstrAnimPrime, "ANIM_MAIN"));

            ltstrAnimSec = GetModelFileList("vani");
            dictModelCoords.Add("Animations - Secondary", CreateModelIndicators(ReadModelFileData(), ltstrAnimSec, "ANIM_SEC"));

            ltstrDungeons = GetModelFileList("dg");
            dictModelCoords.Add("Dungeons", CreateModelIndicators(ReadModelFileData(), ltstrDungeons, "DUNG"));            

            dictModelNamesByType.Add("Buildings", ltstrBuildings);
            dictModelNamesByType.Add("Shapes", ltstrShapes);
            dictModelNamesByType.Add("Trees", ltstrTrees);
            dictModelNamesByType.Add("Grass", ltstrGrass);
            dictModelNamesByType.Add("Animations - Primary", ltstrAnimPrime);
            dictModelNamesByType.Add("Animations - Secondary", ltstrAnimSec);
            dictModelNamesByType.Add("Dungeons", ltstrDungeons);

            Debug.Log("Building Models: " + String.Join(", ", ltstrBuildings));
            Debug.Log("Shape Models: " + String.Join(", ", ltstrShapes));
            Debug.Log("Trees Models: " + String.Join(", ", ltstrTrees));
            Debug.Log("Grass Models: " + String.Join(", ", ltstrGrass));
            Debug.Log("Primary Animation Models: " + String.Join(", ", ltstrAnimPrime));
            Debug.Log("Secondary Animation Models: " + String.Join(", ", ltstrAnimSec));
            Debug.Log("Dungeons Models: " + String.Join(", ", ltstrDungeons));

            for (int nXPos = 0; nXPos < nMapSize; nXPos++) {

                for (int nZPos = 0; nZPos < nMapSize; nZPos++) {

                    ltv3Verts.Add(new Vector3(nXPos,
                                              (float.Parse(abyHeightMap[nZPos / 2 * nSideLen * 2 + nXPos / 2 * 2].ToString()) - 10000) / 50,
                                              nZPos));
                    ltv3Nrmls.Add(-Vector3.forward);

                    if (nXPos > 0 && nZPos > 0) {

                        if (nZPos % 2 == 0) {

                            nTxtrCheckIndex = int.Parse(abyTxtrData[++nTxtrDataIndex].ToString());
                        }

                        if (nZPos == 2) {

                            if (nXPos % 2 == 0) {

                                nTxtrDataIndex = nTxtrDataLastIndex;
                                nTxtrCheckIndex = int.Parse(abyTxtrData[nTxtrDataIndex].ToString());
                            }

                            if (nTxtrCheckIndex < ltmtlGroundTxtrs.Count && nTxtrCheckIndex >= 0) {

                                nTxtrIndex = nTxtrCheckIndex;
                                ltmtlGroundMapList.Add(ltmtlGroundTxtrs[nTxtrIndex]);

                                ltmtlTxtPosInfo.Add(new MaterialPosInfo(nXPos,
                                                                        0,
                                                                        ltmtlGroundTxtrs[nTxtrIndex],
                                                                        nTxtrDataIndex));
                                nMatPosIndex++;
                            }

                            if (nXPos % 2 != 0) {

                                nTxtrDataLastIndex = nTxtrDataIndex;
                            }

                            nTxtrCheckIndex = int.Parse(abyTxtrData[++nTxtrDataIndex].ToString());
                        }

                        if (nZPos >= nMapSize - 1 ||
                            (nTxtrIndex != nTxtrCheckIndex &&
                             nTxtrCheckIndex < ltmtlGroundTxtrs.Count &&
                             nTxtrCheckIndex >= 0)) {

                            ltv3SubmitVerts = ltv3Verts.GetRange(nStartIndex, (nZPos + 1) - nStartIndex);
                            ltv3SubmitVerts.AddRange(ltv3Verts.GetRange(nStartIndex + nMapSize, (nZPos + 1) - nStartIndex));
                            ltv3SubmitNrmls = ltv3Nrmls.GetRange(nStartIndex, (nZPos + 1) - nStartIndex);
                            ltv3SubmitNrmls.AddRange(ltv3Nrmls.GetRange(nStartIndex + nMapSize, (nZPos + 1) - nStartIndex));

                            ltciMeshes.Add(BuildGroundLane(ltv3SubmitVerts,
                                                           GenerateTriangles(ltv3SubmitVerts.Count),
                                                           ltv3SubmitNrmls,
                                                           GenerateUV(ltv3SubmitVerts.Count)));

                            if (nZPos >= nMapSize - 1) { 

                                ltv3Verts.RemoveRange(0, nMapSize);
                                ltv3Nrmls.RemoveRange(0, nMapSize);
                                nStartIndex = 0;

                                ltmtlTxtPosInfo[nMatPosIndex].SetNextPos(nMapSize, nTxtrDataIndex);
                            }
                            else {

                                nStartIndex = nZPos;

                                if (nTxtrCheckIndex < ltmtlGroundTxtrs.Count && nTxtrCheckIndex >= 0) {

                                    nTxtrIndex = nTxtrCheckIndex;
                                }

                                ltmtlTxtPosInfo[nMatPosIndex].SetNextPos(nZPos, nTxtrDataIndex - 1);

                                ltmtlGroundMapList.Add(ltmtlGroundTxtrs[nTxtrIndex]);

                                ltmtlTxtPosInfo.Add(new MaterialPosInfo(nXPos,
                                                                        nZPos,
                                                                        ltmtlGroundTxtrs[nTxtrIndex],
                                                                        nTxtrDataIndex));
                                nMatPosIndex++;
                            }
                        }
                        else if (nMatPosIndex >= 0) {

                            ltmtlTxtPosInfo[nMatPosIndex].AddDataID(nZPos, nTxtrDataIndex);
                        }
                    }
                }
            }

            mrGroundGen.sharedMaterials = ltmtlGroundMapList.ToArray();

            Mesh mhGround = new Mesh();
            mhGround.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;
            mhGround.CombineMeshes(ltciMeshes.ToArray(), false);

            mfGroundInfo.sharedMesh = mhGround;

            Debug.Log("Loading file '" + strFileSelPath + "' complete.");
        }
        catch (Exception exError) {

            Debug.Log("Method: GenerateGround, Action: Loading file, Exception: " + exError.Message + ". Stacktrace: " + exError.StackTrace);
        }
        finally {

            if (fsAccess != null) {

                fsAccess.Close();
            }
        }
    }

    public void SaveFile() {

        FileStream fsAccess = null;
//        int nByteFileWritePos = 0,
//            nModelIndex = 0,
//            nModelInstCount = 0;
 //       byte[] abyFileIndex,
 //              abyFileOut;
//        byte[] abyFileValData;
//        Dictionary<int, List<Vector3[]>> dictModelSetCoords = new Dictionary<int, List<Vector3[]>>();

        try {

            if (strFileSelPath != "") {

                Debug.Log("Saving file '" + strFileSelPath + "' started.");

                byte[] abyFileOut = Encoding.UTF8.GetBytes(new string('\0', abyFileData.Length)),
                       abyFileIndex;

                Buffer.BlockCopy(abyFileData, 0, abyFileOut, 0, 8);

                foreach (Vector3 v3Select in mfGroundInfo.sharedMesh.vertices) {

                    abyHeightMap[(int)v3Select.z / 2 * nSideLen * 2 + (int)v3Select.x / 2 * 2] = HeightTranslate(v3Select.y);
                }

                Buffer.BlockCopy(abyHeightMap, 0, abyFileOut, 8, (int)lHeightMapLen);
                Buffer.BlockCopy(abyTxtrData, 0, abyFileOut, 8 + (int)lHeightMapLen, (int)lTxtrDataLen);

                int nByteFileWritePos = (int)(lHeightMapLen + 8 + lTxtrDataLen),
                    nModelIndex = 0,
                    nModelInstCount = 0;

                abyFileOut = CopyWriteData(abyFileOut, BitConverter.GetBytes(ltmtlGroundTxtrs.Count), nByteFileWritePos);
                nByteFileWritePos += 4;

                foreach (Material mtlSelect in ltmtlGroundTxtrs) {

                    abyFileOut = CopyWriteData(abyFileOut, Encoding.UTF8.GetBytes(mtlSelect.name), nByteFileWritePos);
                    nByteFileWritePos += 260;

                    if (dictGroundSounds.ContainsKey(mtlSelect.name)) {

                        abyFileOut = CopyWriteData(abyFileOut, Encoding.UTF8.GetBytes(dictGroundSounds[mtlSelect.name]), nByteFileWritePos);
                    }

                    nByteFileWritePos += 256;
                }

                abyFileOut = CopyWriteData(abyFileOut, Encoding.UTF8.GetBytes(strWaterFileName), nByteFileWritePos);
                nByteFileWritePos += 256;

                Dictionary<int, List<Vector3[]>> dictModelSetCoords = new Dictionary<int, List<Vector3[]>>();

                foreach (List<string> ltstrModelNames in dictModelNamesByType.Values) {

                    abyFileOut = CopyWriteData(abyFileOut, BitConverter.GetBytes(ltstrModelNames.Count), nByteFileWritePos);
                    nByteFileWritePos += 4;

                    foreach (string strModelName in ltstrModelNames) {

                        abyFileOut = CopyWriteData(abyFileOut, Encoding.UTF8.GetBytes(strModelName), nByteFileWritePos);
                        nByteFileWritePos += 256;

                        dictModelSetCoords.Add(nModelIndex, new List<Vector3[]>());

                        foreach (Dictionary<string, Vector3[]> dictModelInst in dictModelCoords.Values) { 

                            foreach (KeyValuePair<string, Vector3[]> kvpCoords in dictModelInst) { 

                               if (kvpCoords.Key.Split('-')[0] == strModelName) {

                                    dictModelSetCoords[nModelIndex].Add(kvpCoords.Value);
                                    nModelInstCount++;
                               }
                            }
                        }

                        nModelIndex++;
                    }

                    abyFileOut = CopyWriteData(abyFileOut, BitConverter.GetBytes(nModelInstCount), nByteFileWritePos);
                    nByteFileWritePos += 4;

                    foreach (KeyValuePair<int, List<Vector3[]>> kvpCoords in dictModelSetCoords) {

                        abyFileIndex = BitConverter.GetBytes(kvpCoords.Key);

                        foreach (Vector3[] av3Coords in kvpCoords.Value) {

                            abyFileOut = CopyWriteData(abyFileOut, abyFileIndex, nByteFileWritePos);
                            nByteFileWritePos += 4;

                            foreach (Vector3 v3Coord in av3Coords) {

                                abyFileOut = CopyWriteData(abyFileOut, BitConverter.GetBytes(v3Coord.x), nByteFileWritePos);
                                nByteFileWritePos += 4;

                                abyFileOut = CopyWriteData(abyFileOut, BitConverter.GetBytes(v3Coord.y), nByteFileWritePos);
                                nByteFileWritePos += 4;

                                abyFileOut = CopyWriteData(abyFileOut, BitConverter.GetBytes(v3Coord.z), nByteFileWritePos);
                                nByteFileWritePos += 4;
                            }
                        }
                    }

                    dictModelSetCoords.Clear();
                    nModelIndex = 0;
                    nModelInstCount = 0;
                }

                byte[] abyFileValData = Encoding.UTF8.GetBytes(new string('\0', abyFileData.Length - (nByteFileReadPos + 1)));
                Buffer.BlockCopy(abyFileData, nByteFileReadPos, abyFileValData, 0, abyFileData.Length - (nByteFileReadPos + 1));

                abyFileOut = CopyWriteData(abyFileOut, abyFileValData, nByteFileWritePos);

                fsAccess = new FileStream(strFileSelPath, FileMode.Create, FileAccess.Write);
                fsAccess.Write(abyFileOut, 0, abyFileOut.Length);
                fsAccess.Close();

                Debug.Log("Saving '" + strFileSelPath + "' complete.");
            }
            else {

                Debug.Log("No file was loaded for save.");
            }
        }
        catch (Exception exError) {

            Debug.Log("Method: SaveFile, Action: Loading file, Exception: " + exError.Message + ". Stacktrace: " + exError.StackTrace);
        }
        finally {

            if (fsAccess != null) {

                fsAccess.Close();
            }
        }
    }

    byte[] CopyWriteData(byte[] abyWriteData, byte[] abySource, int nOffset) {

//        byte[] abyExpand;

        if (abyWriteData.Length < abySource.Length + nOffset + 1) {

            byte[] abyExpand = Encoding.UTF8.GetBytes(new string('\0', abyWriteData.Length + ((abySource.Length + nOffset + 1) - abyWriteData.Length)));

            Buffer.BlockCopy(abyWriteData, 0, abyExpand, 0, abyWriteData.Length);
            abyWriteData = abyExpand;
        }

        Buffer.BlockCopy(abySource, 0, abyWriteData, nOffset, abySource.Length);

        return abyWriteData;
    }

    CombineInstance BuildGroundLane(List<Vector3> ltv3Verts,
                                    List<int> ltnTriages,
                                    List<Vector3> ltv3Nrmls,
                                    List<Vector2> ltv2UV) {

        Mesh mhGround = new Mesh();
        CombineInstance ciMeshSelect = new CombineInstance();

        mhGround.SetVertices(ltv3Verts);
        mhGround.SetTriangles(ltnTriages, 0);
        mhGround.SetNormals(ltv3Nrmls);
        mhGround.SetUVs(0, ltv2UV);
        mfGroundInfo.sharedMesh = mhGround;

        ciMeshSelect.mesh = mfGroundInfo.sharedMesh;
        ciMeshSelect.transform = mfGroundInfo.transform.localToWorldMatrix;

        return ciMeshSelect;
    }

    /// <summary>
    ///     LoadTextureDXT (Courtesy of user: jeff-smith, site: https://answers.unity.com, 
    ///                     page: https://answers.unity.com/questions/555984/can-you-load-dds-textures-during-runtime.html)
    /// </summary>
    Texture2D LoadTextureDXT(String strFilePathName) {

        FileStream fsAccess = null;
        //        byte[] ddsBytes = null;
        //        int height = ddsBytes[13] * 256 + ddsBytes[12];
        //        int width = ddsBytes[17] * 256 + ddsBytes[16];
        //        int DDS_HEADER_SIZE = 128;
        //        byte[] dxtBytes = new byte[ddsBytes.Length - DDS_HEADER_SIZE];
        Texture2D texture = null;

        try {

            fsAccess = new FileStream(strFilePathName, FileMode.Open, FileAccess.Read);
            byte[] ddsBytes = new byte[fsAccess.Length];

            fsAccess.Read(ddsBytes, 0, (int)fsAccess.Length);
            fsAccess.Close();

            if (ddsBytes[4] != 124)
                throw new Exception("Invalid DDS DXTn texture. Unable to read");  //this header byte should be 124 for DDS image files

            int height = ddsBytes[13] * 256 + ddsBytes[12];
            int width = ddsBytes[17] * 256 + ddsBytes[16];

            int DDS_HEADER_SIZE = 128;
            byte[] dxtBytes = new byte[ddsBytes.Length - DDS_HEADER_SIZE];
            Buffer.BlockCopy(ddsBytes, DDS_HEADER_SIZE, dxtBytes, 0, ddsBytes.Length - DDS_HEADER_SIZE);

            texture = new Texture2D(width, height, TextureFormat.DXT5, false);
            texture.LoadRawTextureData(dxtBytes);
            texture.Apply();
        }
        catch (Exception exError) {

            Debug.Log("Method: LoadTextureDXT, Action: Loading file, '" + strFilePathName + "', failed. Exception: " + exError.Message);
            throw exError;
        }
        finally {

            if (fsAccess != null) {

                fsAccess.Close();
            }
        }

        return (texture);
    }

    List<int> GenerateTriangles(int nSize) {

        List<int> ltnTriages = new List<int>();
        int nSectionSize = nSize / 2;

        try {

            for (int nCounter = nSectionSize + 1; nCounter < nSize; nCounter++) {

                ltnTriages.Add(nCounter - 1);
                ltnTriages.Add((nCounter - nSectionSize) - 1);
                ltnTriages.Add(nCounter - nSectionSize);
                ltnTriages.Add(nCounter - nSectionSize);
                ltnTriages.Add(nCounter);
                ltnTriages.Add(nCounter - 1);
            }
        }
        catch (Exception exError) {

            throw exError;
        }

        return ltnTriages;
    }

    List<Vector2> GenerateUV(int nVertListLen) {

        List<Vector2> ltv2UV = new List<Vector2>();
        int nSectionSize = nVertListLen / 2;
        float fRowVal = 0;
        float fTileVal = 0;

        try {

            for (int nCounter = 0; nCounter < nVertListLen; nCounter++) {

                if (nCounter == nSectionSize) {

                    fRowVal = 1;
                    fTileVal = 0;
                }

                ltv2UV.Add(new Vector2(fRowVal, fTileVal));
                fTileVal += (float)0.5;
            }
        }
        catch (Exception exError) {

            throw exError;
        }

        return ltv2UV;
    }

    List<string> GetModelFileList(string strFileType) {

        byte[] abyFileSectData = new byte[256];
        int nFileTypeLen = strFileType.Length;
        int nFileCount = BitConverter.ToInt32(abyFileData, nByteFileReadPos);
        string strFoundText = "";
        List<string> ltstrFileList = new List<string>();

        nByteFileReadPos += 4;

        for (int nCounter = 0; nCounter < nFileCount; nCounter++) {

            Buffer.BlockCopy(abyFileData, nByteFileReadPos, abyFileSectData, 0, 256);

            strFoundText = Encoding.UTF8.GetString(abyFileSectData);
            ltstrFileList.Add(strFoundText.Substring(0, strFoundText.IndexOf(".") + nFileTypeLen + 1));

            nByteFileReadPos += 256;
        }

        return ltstrFileList;
    }

    Dictionary<int, List<Vector3[]>> ReadModelFileData() {

        UInt32 nModelFileCounts = 0;
        int nFileIndex = 0;
        Dictionary<int, List<Vector3[]>> dictCoods = new Dictionary<int, List<Vector3[]>>();

        try {

            nModelFileCounts = BitConverter.ToUInt32(abyFileData, nByteFileReadPos);

            for (int nCounter = 0; nCounter < nModelFileCounts; nCounter++) {

                nFileIndex = (int)BitConverter.ToUInt32(abyFileData, nByteFileReadPos += 4);

                if (!dictCoods.ContainsKey(nFileIndex)) {

                    dictCoods.Add(nFileIndex, new List<Vector3[]>());
                }

                dictCoods[nFileIndex].Add(new Vector3[] {
                    new Vector3(BitConverter.ToSingle(abyFileData, nByteFileReadPos += 4),
                                BitConverter.ToSingle(abyFileData, nByteFileReadPos += 4),
                                BitConverter.ToSingle(abyFileData, nByteFileReadPos += 4)),
                    new Vector3(BitConverter.ToSingle(abyFileData, nByteFileReadPos += 4),
                                BitConverter.ToSingle(abyFileData, nByteFileReadPos += 4),
                                BitConverter.ToSingle(abyFileData, nByteFileReadPos += 4)),
                    new Vector3(BitConverter.ToSingle(abyFileData, nByteFileReadPos += 4),
                                BitConverter.ToSingle(abyFileData, nByteFileReadPos += 4),
                                BitConverter.ToSingle(abyFileData, nByteFileReadPos += 4))
                });
            }

            nByteFileReadPos += 4;
        }
        catch (Exception exError) {

            throw exError;
        }

        return dictCoods;
    }

    Dictionary<string, Vector3[]> CreateModelIndicators(Dictionary<int, List<Vector3[]>> dictModelInfo,
                                                        List<string> ltstrNames,
                                                        string strType) {

        string strModelName = "";
        int nFileNum = 0;
        int nFileNameNum = 0;
        Dictionary<string, Vector3[]> dictSetModelCoords = new Dictionary<string, Vector3[]>();

        foreach (KeyValuePair<int, List<Vector3[]>> kvpModelData in dictModelInfo) {

            foreach (Vector3[] av3Coords in kvpModelData.Value) {
                
                strModelName = ltstrNames[kvpModelData.Key] + "-" + ++nFileNameNum + "-" + strType + ++nFileNum;

                PlaceModelIndicators(strModelName, av3Coords[0]);

                dictSetModelCoords.Add(strModelName, av3Coords);
            }

            nFileNameNum = 0;
        }

        return dictSetModelCoords;
    }

    void PlaceModelIndicators(string strName, Vector3 v3Pos) {

        GameObject goIndicator = GameObject.Find("Model Indicator");
        GameObject goCloneHolder = new GameObject(strName);
        Mesh mhOrig = goIndicator.GetComponent<MeshFilter>().sharedMesh;
        Mesh mhClone = new Mesh();
        Transform tfCloneLoc = goCloneHolder.transform;

        mhClone.vertices = mhOrig.vertices;
        mhClone.triangles = mhOrig.triangles;
        mhClone.uv = mhOrig.uv;
        mhClone.normals = mhOrig.normals;
        mhClone.colors = mhOrig.colors;
        mhClone.tangents = mhOrig.tangents;

        goCloneHolder.AddComponent<MeshFilter>().sharedMesh = mhClone;
        goCloneHolder.AddComponent<MeshRenderer>().sharedMaterials = goIndicator.GetComponent<MeshRenderer>().sharedMaterials;
        tfCloneLoc.position = new Vector3(v3Pos.x, 0, v3Pos.z);
        tfCloneLoc.rotation = goIndicator.transform.rotation;
        tfCloneLoc.localScale = goIndicator.transform.localScale;
    }

    public List<string> GetOrigModelNames() {

        List<string> ltstrNames = new List<string>();

        foreach (List<string> ltstrModelList in dictModelNamesByType.Values) {

            ltstrNames.AddRange(ltstrModelList);
        }

        return ltstrNames;
    }

    public List<string> GetModelTypes() {

        List<string> ltstrNames = new List<string>();

        ltstrNames.AddRange(dictModelNamesByType.Keys);

        return ltstrNames;
    }

    public float SetGroundHeightByCoords(Vector3 v3Set) {

        Vector3[] av3Vertices = mfGroundInfo.sharedMesh.vertices;
        int nCoordVal = (int)v3Set.z / 2 * nSideLen * 2 + (int)v3Set.x / 2 * 2;
        int nVertTotal = av3Vertices.Length,
            nCounter = 0;

        try {

            if (v3Set.x >= 0 && v3Set.x < nMapSize &&
                v3Set.z >= 0 && v3Set.z < nMapSize &&
                HeightTranslate(v3Set.y) >= 0 && HeightTranslate(v3Set.y) <= 255) {

                for (nCounter = 0; nCounter < nVertTotal; nCounter++) {

                    if (nCoordVal == (int)av3Vertices[nCounter].z / 2 * nSideLen * 2 + (int)av3Vertices[nCounter].x / 2 * 2) {

                        av3Vertices[nCounter].y = v3Set.y;
                        Debug.Log("Updated X: " + av3Vertices[nCounter].x + ", Z: " + av3Vertices[nCounter].z + " to " + v3Set.y);
                    }
                }

                mfGroundInfo.sharedMesh.SetVertices(av3Vertices);
            }
            else {

                Debug.Log("Coordindates - X: " + v3Set.x + ", Z: " + v3Set.z + " or height value: " + v3Set.y + " are invalid.");
                v3Set.y = GetGroundHeightByCoords(v3Set.x, v3Set.z);
            }
        }
        catch (Exception exError) {

            Debug.Log("Coordindates - X: " + v3Set.x + ", Z: " + v3Set.z + " or height value: " + v3Set.y + " are invalid.");
            v3Set.y = GetGroundHeightByCoords(v3Set.x, v3Set.z);
        }

        return v3Set.y;
    }

    public string SetGroundTextureByCoords(float fSetXPos, float fSetZPos, string strTxtrFileName) {

        int nXPos = (int)fSetXPos,
            nZPos = (int)fSetZPos,
            nTxtrIndex = -1;
        bool boolNotFound = true;

        if (nXPos >= 0 && nXPos < nMapSize &&
            nZPos >= 0 && nZPos < nMapSize &&
            strTxtrFileName != "") {
        
            strTxtrFileName = strTxtrFileName.Replace(".tga", ".dds");

            foreach (Material mtlSelect in ltmtlGroundTxtrs) {

                nTxtrIndex++;

                if (mtlSelect.name == strTxtrFileName) {
                    boolNotFound = false;
                    break;    
                }
            }

            if (boolNotFound) {

                ltmtlGroundTxtrs.Add(CreateMaterial(strTxtrFileName));
                nTxtrIndex = ltmtlGroundTxtrs.Count - 1;
                boolNotFound = true;
            }

            foreach (MaterialPosInfo mpiSelect in ltmtlTxtPosInfo) {

                if (mpiSelect.IsInArea(nXPos, nZPos)) {

                    strTxtrFileName = ltmtlGroundTxtrs[nTxtrIndex].name;
                    abyTxtrData[mpiSelect.GetDataIDByCoords(nZPos)] = Convert.ToByte(nTxtrIndex);

                    boolNotFound = false;
                    break;
                }
            }

            if (boolNotFound) {

                Debug.Log("Coordindates - X: " + nXPos + ", Z: " + nZPos + " or texture filename: '" + strTxtrFileName + "' are invalid.");
            }
        }
        else {

            Debug.Log("Coordindates - X: " + nXPos + ", Z: " + nZPos + " or texture filename: '" + strTxtrFileName + "' are invalid.");
        }

        return strTxtrFileName;
    }

    public float GetGroundHeightByCoords(float fSetXPos, float fSetZPos) {

        int nXPos = (int)fSetXPos,
            nZPos = (int)fSetZPos;
        float fYPos = -1000000;

        if (nXPos >= 0 && nXPos < nMapSize &&
            nZPos >= 0 && nZPos < nMapSize) {

            foreach (Vector3 v3Select in mfGroundInfo.sharedMesh.vertices) {

                if (v3Select.x == nXPos &&
                    v3Select.z == nZPos) {

                    fYPos = v3Select.y;
                    break;
                }
            }
        }
        else {

            Debug.Log("Coordindates - X: " + nXPos + ", Z: " + nZPos + " are invalid.");
        }

        return fYPos;
    }

    public string GetGroundTextureByCoords(float fSetXPos, float fSetZPos) {

        int nXPos = (int)fSetXPos,
            nZPos = (int)fSetZPos;
        string strTxtrFileName = "";

        if (nXPos >= 0 && nXPos < nMapSize &&
            nZPos >= 0 && nZPos < nMapSize) {

            foreach (MaterialPosInfo mpiSelect in ltmtlTxtPosInfo) {

                if (mpiSelect.IsInArea(nXPos, nZPos)) {
                    
                    strTxtrFileName = ltmtlGroundTxtrs[abyTxtrData[mpiSelect.GetDataIDByCoords(nZPos)]].name;
                    break;
                }
            }
        }
        else {

            Debug.Log("Coordindates - X: " + nXPos + ", Z: " + nZPos + " are invalid.");
        }

        return strTxtrFileName;
    }

    public Vector3[] ModelCoordsByName(string strName) {

        Vector3[] v3Ret = new Vector3[] { new Vector3(), new Vector3(), new Vector3() };

        foreach (Dictionary<string, Vector3[]> dictCoords in dictModelCoords.Values) {

            if (dictCoords.ContainsKey(strName)) {

                v3Ret = dictCoords[strName];
                break;
            }
        }

        return v3Ret;
    }

    public GameObject CreateModelCoordsByName(string strOrgModelName, string strModelType, Vector3[] av3Coords) {

        Dictionary<string, Vector3[]> dictDistList = null;
        int nModelCount = 0;
//        string strNewModelName = "";
        GameObject goNew = null;

        if (strOrgModelName.IndexOf('.') < 0) {

            switch (strModelType) {
                
                case "Buildings": { 
                
                    strOrgModelName = strOrgModelName.Trim() + ".smod";
                    break;
                }
                case "Shapes": { 
                
                    strOrgModelName = strOrgModelName.Trim() + ".smod";
                    break;
                }
                case "Trees": {
                
                    strOrgModelName = strOrgModelName.Trim() + ".smod";
                    break;
                }
                case "Grass": { 
                
                    strOrgModelName = strOrgModelName.Trim() + ".smod";
                    break;
                }
                case "Animations - Primary": { 
                
                    strOrgModelName = strOrgModelName.Trim() + ".vani";
                    break;
                }
                case "Animations - Secondary": { 
                
                    strOrgModelName = strOrgModelName.Trim() + ".vani";
                    break;
                }
                case "Dungeons": { 
                
                    strOrgModelName = strOrgModelName.Trim() + ".dg";
                    break;
                }
                default: {

                    Debug.Log("Warning: New model '" + strOrgModelName + "' doesn't have a file type or set mode type, may cause issues.");
                    break;
                }
            }
        }

        foreach (KeyValuePair<string, Dictionary<string, Vector3[]>> kvpModelData in dictModelCoords) {

            if (kvpModelData.Key == strModelType) {

                dictDistList = kvpModelData.Value;

                foreach (KeyValuePair<string, Vector3[]> kvpModelInfo in kvpModelData.Value) {

                    if (kvpModelInfo.Key.StartsWith(strOrgModelName)) {

                        nModelCount++;
                    }
                }

                break;
            }
        }

        if (dictDistList != null) {

            string strNewModelName = strOrgModelName + "-" + ++nModelCount + "-ADDED" + (dictDistList.Count + 1);

            if (!dictModelNamesByType[strModelType].Contains(strOrgModelName)) {

                dictModelNamesByType[strModelType].Add(strOrgModelName);
            }

            dictDistList.Add(strNewModelName, (Vector3[])av3Coords.Clone());
            PlaceModelIndicators(strNewModelName, av3Coords[0]);
            goNew = GameObject.Find(strNewModelName);

            Debug.Log("Creation of new model '" + strNewModelName + "' complete.");
        }
        else {

            Debug.Log("Creation of new model '" + strOrgModelName + "' failed.");
        }

        return goNew;
    }

    public GameObject AddModelCoordsByName(string strOrgModelName, Vector3[] av3Coords) {

        Dictionary<string, Vector3[]> dictDistList = null;
        int nModelCount = 0;
//        string strNewModelName = "";
        GameObject goNew = null;

        foreach (Dictionary<string, Vector3[]> dictCoords in dictModelCoords.Values) {

            foreach (KeyValuePair<string, Vector3[]> kvpModelInfo in dictCoords) {

                if (kvpModelInfo.Key.StartsWith(strOrgModelName)) {

                    if (dictDistList == null) {

                        dictDistList = dictCoords;
                    }

                    nModelCount++;
                }
            }
        }

        if (dictDistList == null) { 
        
            foreach (List<string> ltstrModelNames in dictModelNamesByType.Values) { 
            
                foreach (string strModelName in ltstrModelNames) {

                    foreach (Dictionary<string, Vector3[]> dictCoords in dictModelCoords.Values) {

                        foreach (KeyValuePair<string, Vector3[]> kvpModelInfo in dictCoords) {

                            if (kvpModelInfo.Key.StartsWith(strModelName) &&
                                dictDistList == null) {

                                dictDistList = dictCoords;
                                break;
                            }
                        }

                        if (dictDistList != null) {

                            break;
                        }
                    }

                    if (dictDistList != null) {

                        break;
                    }
                }

                if (dictDistList != null) {

                    break;
                }
            }
        }

        if (dictDistList != null) {

            string strNewModelName = strOrgModelName + "-" + ++nModelCount + "-ADDED" + (dictDistList.Count + 1);

            dictDistList.Add(strNewModelName, (Vector3[])av3Coords.Clone());
            PlaceModelIndicators(strNewModelName, av3Coords[0]);
            goNew = GameObject.Find(strNewModelName);

            Debug.Log("New model instance '" + strNewModelName + "' created.");
        }

        return goNew;
    }

    public void UpdateModelCoordsByName(string strName, Vector3[] av3Coords) {
        
        bool boolNotFound = true;

        foreach (Dictionary<string, Vector3[]> dictCoords in dictModelCoords.Values) {

            if (dictCoords.ContainsKey(strName)) {

                dictCoords[strName] = (Vector3[])av3Coords.Clone();
                GameObject.Find(strName).transform.position = new Vector3(av3Coords[0].x, 0, av3Coords[0].z);
                boolNotFound = false;

                Debug.Log("Moved model instance '" + strName + "' to X: " + av3Coords[0].x + ", Z: " + av3Coords[0].z);
                break;
            }
        }

        if (boolNotFound) {

            Debug.Log("Model, '" + strName + "', not found for update.");
        }
    }

    public void RemoveModelIndicatorsByName(string strName) {

        GameObject goSelected = GameObject.Find(strName);

        if (goSelected != null) {

            goSelected.GetComponent<MeshFilter>().sharedMesh.Clear();
            GameObject.Destroy(goSelected.GetComponent<MeshFilter>().sharedMesh);
            Destroy(goSelected);
        }
        else {

            Debug.Log("Removal of model instance '" + strName + "' failed.");
        }
    }

    public void RemoveModelIndicators() {

        Debug.Log("Removing model indicators started.");

        foreach (Dictionary<string, Vector3[]> dictModelList in dictModelCoords.Values) {

            foreach (string strModelName in dictModelList.Keys) {

                RemoveModelIndicatorsByName(strModelName);
            }
        }

        Debug.Log("Removing model indicators complete.");
    }

    public void DeleteModelCoordsByName(string strName) {

        bool boolNotFound = true;

        foreach (Dictionary<string, Vector3[]> dictCoords in dictModelCoords.Values) {

            if (dictCoords.ContainsKey(strName)) {

                dictCoords.Remove(strName);
                boolNotFound = false;

                Debug.Log("Removed model instance '" + strName + "'.");
                break;
            }
        }

        if (boolNotFound) {

            Debug.Log("Model, '" + strName + "', not found for deletion.");
        }
    }

    public void ShowModelIndicators() {

        RemoveModelIndicators();

        Debug.Log("Rendering model indicators started.");

        foreach (Dictionary<string, Vector3[]> dictCoords in dictModelCoords.Values) {

            foreach (KeyValuePair<string, Vector3[]> kvpModelInfo in dictCoords) {

                PlaceModelIndicators(kvpModelInfo.Key, kvpModelInfo.Value[0]);
            }
        }

        Debug.Log("Rendering model indicators completed.");
    }

    public void ShowModelIndicatorsByType(string strModelType) {

        RemoveModelIndicators();

        Debug.Log("Rendering model indicators for type '" + strModelType + "' started.");

        foreach (KeyValuePair<string, Dictionary<string, Vector3[]>> kvpModelData in dictModelCoords) {

            if (strModelType == kvpModelData.Key) {

                foreach (KeyValuePair<string, Vector3[]> kvpModelInfo in kvpModelData.Value) {

                    PlaceModelIndicators(kvpModelInfo.Key, kvpModelInfo.Value[0]);
                }

                break;
            }
        }

        Debug.Log("Rendering model indicators for type '" + strModelType + "' complete.");
    }

    public int GetLastIndex {

        get {

            int nIndex = nMapSize;

            if (nMapSize > 0) {

                nIndex = nMapSize - 1;
            }

            return nIndex;
        }
    }

    private byte HeightTranslate(float fVal) {

        byte byVal = 0;

        try {

            byVal = Convert.ToByte(Math.Round(((fVal - 100) * 50) + 10000));
        }
        catch (Exception exError) {

            throw exError;
        }

        return byVal;
    }

    private Material CreateMaterial(string strTxtrPath) {

        Material mtlCreate = new Material(Shader.Find("Standard"));
        mtlCreate.name = strTxtrPath;
        mtlCreate.mainTexture = LoadTextureDXT("Assets/Textures/Environment/" + strTxtrPath);

        return mtlCreate;
    }

    internal class MaterialPosInfo {

        private int nXRow = -1,
                    nStartXPos = -1,
                    nStartZPos = -1,
                    nNextZPos = -1;
        private Material matSelect;
        private Dictionary<int, int> dictDataIndices = new Dictionary<int, int>();

        public MaterialPosInfo(int nSetXRow,
                               int nSetStartZPos,
                               Material matSetSelect,
                               int nSetDataIndex) {

            nXRow = nSetXRow;
            nStartXPos = nSetXRow - 1;
            nStartZPos = nSetStartZPos;
            matSelect = matSetSelect;
            dictDataIndices.Add(nSetStartZPos, nSetDataIndex);
        }

        public void SetNextPos(int nSetNextZPos, int nDataID) {

            nNextZPos = nSetNextZPos;
            AddDataID(nSetNextZPos, nDataID);
        }

        public bool IsInArea(int nXPos,
                             int nZPos) {

            return nXPos == nXRow &&
                   nZPos >= nStartZPos &&
                   nZPos < nNextZPos;
        }

        public int GetDataIDByCoords(int nZPos) {

            int nDataID = -1;

            if (dictDataIndices.ContainsKey(nZPos)) {

                nDataID = dictDataIndices[nZPos];
            }
            else { 
            
                foreach (KeyValuePair<int, int> kvpTxtrData in dictDataIndices) { 
                
                    if (kvpTxtrData.Key <= nZPos) {

                        nDataID = kvpTxtrData.Value;
                    }
                    else {

                        break;
                    }
                }
            }

            return nDataID;
        }

        public void AddDataID(int nZPos, int nDataID) {

            if (!dictDataIndices.ContainsKey(nZPos)) {

                dictDataIndices.Add(nZPos, nDataID);
            }
        }

        public bool HasDataID(int nDataID) {

            return dictDataIndices.ContainsValue(nDataID);
        }
        
        public Material Material {

            get {

                return matSelect;
            }
        }
    }
}

#if UNITY_EDITOR_WIN
public class WLDFileUpdater : EditorWindow {

    private Vector3 v3Vertex = new Vector3();
    private Vector3[] av3ModelSel = new Vector3[] { new Vector3(), new Vector3(), new Vector3() };
    private Vector2 v2MainSelPos = new Vector2(),
                    v2FileSelPos = new Vector2(),
                    v2AddModelSelPos = new Vector2(),
                    v2SelModelSelPos = new Vector2(),
                    v2NewModelTypeSelPos = new Vector2(),
                    v2TxtrSelPos = new Vector2();
    private GameObject goSelected = null;
    private string strCreateModelName = "",
                   strTxtrCurrent = "";
    private Vector3 v3CamPos = new Vector3(),
                    v3CamLookAt = new Vector3();

    [MenuItem("Plugins/WLD File Updater")]
    static void Init() {

        EditorWindow.GetWindow(typeof(WLDFileUpdater), false, "WLD File Updater").Show();
    }

    void OnGUI() {
    
        WLDFileLoader wdlLoader = GameObject.Find("Ground Generator").GetComponent<WLDFileLoader>();
        string[] astrModelFileNames = Directory.GetFiles("Assets/Model/");
        string[] astrTxtrAvail = Directory.GetFiles("Assets/Textures/Environment/");
        string strWldFilePath = "";
        GUIStyle gsLabel = GUI.skin.GetStyle("Label");
        GUILayoutOption gloLabelHeight = GUILayout.Height(25),
                        gloSelHeight = GUILayout.Height(75);

        gsLabel.alignment = TextAnchor.MiddleLeft;
        
        v2MainSelPos = GUILayout.BeginScrollView(v2MainSelPos);
        GUILayout.BeginVertical("box");
        GUILayout.Label("Select WLD File:", gsLabel, gloLabelHeight);
        v2FileSelPos = GUILayout.BeginScrollView(v2FileSelPos, gloSelHeight);
//      Transform tfCamera = UnityEditor.SceneView.lastActiveSceneView.camera.transform;

        foreach (string strFilePath in astrModelFileNames) {
        
            if (strFilePath.EndsWith(".wld")) {
            
                if (GUILayout.Button(strFilePath.Split('/')[strFilePath.Split('/').Length - 1])) {
                
                    strWldFilePath = strFilePath;
                }
            }
        }

        GUILayout.EndScrollView();
            
        if (GUILayout.Button("SAVE")) {
                
            wdlLoader.SaveFile();
        }

        GUILayout.EndVertical();

        if (strWldFilePath != "") {
        
            wdlLoader.GenerateGround(strWldFilePath);
        }

        GUILayout.BeginVertical("box");
        GUILayout.Label("Edit Height by Position:", gsLabel, gloLabelHeight);
        v3Vertex.x = float.Parse(EditorGUILayout.TextField("Find X (0 - " + wdlLoader.GetLastIndex + "): ", v3Vertex.x.ToString()));
        v3Vertex.z = float.Parse(EditorGUILayout.TextField("Find Z (0 - " + wdlLoader.GetLastIndex + "): ", v3Vertex.z.ToString()));
        
        if (GUILayout.Button("FIND")) {
        
            v3Vertex.y = wdlLoader.GetGroundHeightByCoords(v3Vertex.x, v3Vertex.z);

            if (v3Vertex.y == -1000000) {
            
                Debug.Log("No height information for position - X: " + v3Vertex.x + ", Z: " + v3Vertex.z);
            }

            v3CamPos = new Vector3(v3Vertex.x, v3Vertex.y + 110, v3Vertex.z);
            v3CamLookAt = v3Vertex;
        }

        v3Vertex.y = float.Parse(EditorGUILayout.TextField("Set Height (Y): ", v3Vertex.y.ToString()));
        
        if (GUILayout.Button("UPDATE")) {
        
            v3Vertex.y = wdlLoader.SetGroundHeightByCoords(v3Vertex);
        }

        GUILayout.EndVertical();

        if (goSelected != Selection.activeGameObject) {
        
            goSelected = Selection.activeGameObject;
            av3ModelSel = wdlLoader.ModelCoordsByName(goSelected.name);
        }

        GUILayout.BeginVertical("box");
        GUILayout.Label("Edit Selected Model: (X and Z positions visualized)", gsLabel, gloLabelHeight);
            
        GUILayout.Label("Position:", gsLabel, gloLabelHeight);
        av3ModelSel[0].x = EditorGUILayout.FloatField("X:", av3ModelSel[0].x);
        av3ModelSel[0].y = EditorGUILayout.FloatField("Y:", av3ModelSel[0].y);
        av3ModelSel[0].z = EditorGUILayout.FloatField("Z:", av3ModelSel[0].z);

        GUILayout.Label("Rotation:", gsLabel, gloLabelHeight);
        av3ModelSel[1].x = EditorGUILayout.FloatField("X:", av3ModelSel[1].x);
        av3ModelSel[1].y = EditorGUILayout.FloatField("Y:", av3ModelSel[1].y);
        av3ModelSel[1].z = EditorGUILayout.FloatField("Z:", av3ModelSel[1].z);

        GUILayout.Label("Scale:", gsLabel, gloLabelHeight);
        av3ModelSel[2].x = EditorGUILayout.FloatField("X:", av3ModelSel[2].x);
        av3ModelSel[2].y = EditorGUILayout.FloatField("Y:", av3ModelSel[2].y);
        av3ModelSel[2].z = EditorGUILayout.FloatField("Z:", av3ModelSel[2].z);
        
        if (GUILayout.Button("UPDATE MODEL") && goSelected != null) {

            wdlLoader.UpdateModelCoordsByName(goSelected.name, av3ModelSel);
        }
        
        if (GUILayout.Button("DELETE MODEL") && 
            goSelected != null && 
            EditorUtility.DisplayDialog("Delete model?", "Delete model '" + goSelected.name + "'?", "DELETE")) {

            wdlLoader.RemoveModelIndicatorsByName(goSelected.name);
            wdlLoader.DeleteModelCoordsByName(goSelected.name);
            goSelected = null;
            av3ModelSel = new Vector3[] { new Vector3(), new Vector3(), new Vector3() };
        }

        GUILayout.EndVertical();

        GUILayout.BeginVertical("box");
        GUILayout.Label("Add Model Instance:", gsLabel,gloLabelHeight);
        v2AddModelSelPos = GUILayout.BeginScrollView(v2AddModelSelPos, gloSelHeight);

        foreach (string strModelName in wdlLoader.GetOrigModelNames()) {
        
            if (GUILayout.Button(strModelName)) {

                av3ModelSel[0] = new Vector3();
                av3ModelSel[1] = new Vector3();
                av3ModelSel[2] = new Vector3(0, 1, 0);
                Selection.activeGameObject = wdlLoader.AddModelCoordsByName(strModelName, av3ModelSel);

                if (Selection.activeGameObject == null) {
                
                    Debug.Log("No instances of '" + strModelName + "' exists to copy. Select the type under the 'New Model Name' textbox to create.");
                    strCreateModelName = strModelName;
                }
            }
        }

        GUILayout.EndScrollView();
        GUILayout.EndVertical();

        GUILayout.BeginVertical("box");
        strCreateModelName = EditorGUILayout.TextField("New Model Name: ", strCreateModelName);
        v2NewModelTypeSelPos = GUILayout.BeginScrollView(v2NewModelTypeSelPos, gloSelHeight);

        foreach (string strModelType in wdlLoader.GetModelTypes()) {
        
            if (GUILayout.Button("Create " + strModelType) && strCreateModelName != "") {
            
                av3ModelSel[0] = new Vector3();
                av3ModelSel[1] = new Vector3();
                av3ModelSel[2] = new Vector3(0, 1, 0);
                Selection.activeGameObject = wdlLoader.CreateModelCoordsByName(strCreateModelName, strModelType, av3ModelSel);
                strCreateModelName = "";
            }
        }

        GUILayout.EndScrollView();
        GUILayout.EndVertical();

        GUILayout.BeginVertical("box");
        GUILayout.Label("Show Models by Type:", gsLabel, gloLabelHeight);
        v2SelModelSelPos = GUILayout.BeginScrollView(v2SelModelSelPos, gloSelHeight);

        if (GUILayout.Button("Show All")) {

            Selection.activeGameObject = null;
            av3ModelSel = new Vector3[] { new Vector3(), new Vector3(), new Vector3() };
            wdlLoader.ShowModelIndicators();
        }

        if (GUILayout.Button("Hide All")) {

            Selection.activeGameObject = null;
            av3ModelSel = new Vector3[] { new Vector3(), new Vector3(), new Vector3() };
            wdlLoader.RemoveModelIndicators();
        }

        foreach (string strModelType in wdlLoader.GetModelTypes()) {
        
            if (GUILayout.Button(strModelType)) {
            
                Selection.activeGameObject = null;
                av3ModelSel = new Vector3[] { new Vector3(), new Vector3(), new Vector3() };
                wdlLoader.ShowModelIndicatorsByType(strModelType);
            }
        }

        GUILayout.EndScrollView();
        GUILayout.EndVertical();

        GUILayout.BeginVertical("box");
        GUILayout.Label("Edit Texture Section by Position: (4 Squares)", gsLabel, gloLabelHeight);
        v3Vertex.x = float.Parse(EditorGUILayout.TextField("Find X Row (1 - " + wdlLoader.GetLastIndex + "): ", v3Vertex.x.ToString()));
        v3Vertex.z = float.Parse(EditorGUILayout.TextField("Find Z (0 - " + wdlLoader.GetLastIndex + "): ", v3Vertex.z.ToString()));
        
        if (GUILayout.Button("FIND TEXTURE SECTION")) {
        
            v3Vertex.y = wdlLoader.GetGroundHeightByCoords(v3Vertex.x, v3Vertex.z);
            strTxtrCurrent = wdlLoader.GetGroundTextureByCoords(v3Vertex.x, v3Vertex.z);

            if (v3Vertex.y == -1000000) {
            
                Debug.Log("No texture information for position - X: " + v3Vertex.x + ", Z: " + v3Vertex.z);
            }

            v3CamPos = new Vector3(v3Vertex.x, v3Vertex.y + 110, v3Vertex.z);
            v3CamLookAt = v3Vertex;
        }
      
        EditorGUILayout.TextField("Texture: ", strTxtrCurrent);

        GUILayout.Label("Select Texture:", gsLabel, gloLabelHeight);
        v2TxtrSelPos = GUILayout.BeginScrollView(v2TxtrSelPos, gloSelHeight);

        foreach (string strTxtrSel in astrTxtrAvail) {
        
            if (!strTxtrSel.EndsWith(".meta") && 
                GUILayout.Button(strTxtrSel.Split('/')[strTxtrSel.Split('/').Length - 1])) {
            
                strTxtrCurrent = wdlLoader.SetGroundTextureByCoords(v3Vertex.x, 
                                                                    v3Vertex.z, 
                                                                    strTxtrSel.Split('/')[strTxtrSel.Split('/').Length - 1]);
            }
        }

        GUILayout.EndScrollView();
        GUILayout.EndVertical();
        GUILayout.EndScrollView();

        if (v3CamPos != Vector3.zero && UnityEditor.SceneView.lastActiveSceneView != null) {
            Transform tfCamera = UnityEditor.SceneView.lastActiveSceneView.camera.transform;
            tfCamera.position = v3CamPos;
            tfCamera.LookAt(v3CamLookAt);
            UnityEditor.SceneView.lastActiveSceneView.AlignViewToObject(tfCamera);
            v3CamPos = Vector3.zero;
            v3CamLookAt = Vector3.zero;
        }
    }

     void OnSelectionChange() {

        Repaint();
     }
}
#endif