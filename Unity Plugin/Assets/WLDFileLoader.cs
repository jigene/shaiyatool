using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;

/* TODO - When writing data back to file, work out the math for updating ground textures without removing sound filenames */

public class WLDFileLoader : MonoBehaviour {

    private MeshFilter mfGroundInfo;
    private int nByteFilePos = 0;

    // Start is called before the first frame update
    void Start() {

        FileStream fsAccess = null;
        byte[] abyFileData;
        byte[] abyHeightMap;
        byte[] abyTxtrData;
        byte[] abyFileSectData;
        int nMapSize = 0;
        long lHeightMapLen = 0;
        long lTxtrDataLen = 0;
        int nFileCount = 0;
        Material mtlCreate;
        List<Material> ltmtlGroundTxtrs = new List<Material>();
        string strFoundText = "";
        string strGroundTxtrFileNames = "";
        List<string> ltstrBuildings;
        List<string> ltstrShapes;
        List<string> ltstrTrees;
        List<string> ltstrGrass;
        List<string> ltstrAnimPrime;
        List<string> ltstrAnimSec;
        List<string> ltstrDungeons;
        int nSideLen = 0;
        int nTxtrIndex = 0;
        int nTxtrCheckIndex = 0;
        int nTxtrDataIndex = -1;
        int nStartIndex = 0;
        List<Material> ltmtlGroundMapList = new List<Material>();
        List<Vector3> ltv3Verts = new List<Vector3>();
        List<Vector3> ltv3Nrmls = new List<Vector3>();
        List<Vector3> ltv3SubmitVerts;
        List<Vector3> ltv3SubmitNrmls;
        List<CombineInstance> ltciMeshes = new List<CombineInstance>();

        MeshRenderer mrGroundGen = gameObject.AddComponent<MeshRenderer>();

        mfGroundInfo = gameObject.AddComponent<MeshFilter>();

        try {

            fsAccess = new FileStream("Assets/Models/0.wld", FileMode.Open, FileAccess.Read);
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

            nByteFilePos = (int)(lHeightMapLen + 8 + lTxtrDataLen);
            nFileCount = BitConverter.ToInt32(abyFileData, nByteFilePos);

            nByteFilePos += 4;

            abyFileSectData = new byte[256];

            for (int nCounter = 0; nCounter < nFileCount; nCounter++) {

                Buffer.BlockCopy(abyFileData, nByteFilePos, abyFileSectData, 0, 256);

                if (strGroundTxtrFileNames != "") {

                    strGroundTxtrFileNames += ", ";
                }

                strFoundText = Encoding.UTF8.GetString(abyFileSectData);
                strGroundTxtrFileNames += strFoundText.Substring(0, strFoundText.IndexOf(".") + 4);

                mtlCreate = new Material(Shader.Find("Standard"));
                mtlCreate.mainTexture = LoadTextureDXT("Assets/Textures/Environment/" +
                                                       strFoundText.Substring(0,
                                                                              strFoundText.IndexOf(".") + 4)
                                                                                          .Replace(".tga", ".dds"));

                ltmtlGroundTxtrs.Add(mtlCreate);
                strFoundText = "";

                nByteFilePos += 516;
            }

            Debug.Log("Map Size: " + nMapSize);
            Debug.Log("Ground Textures: " + strGroundTxtrFileNames);

            nByteFilePos += 256;

            ltstrBuildings = GetModelFileList(abyFileData, "smod");
            CreateModelIndicators(ReadModelFileData(abyFileData), ltstrBuildings, "BLG");
            ltstrShapes = GetModelFileList(abyFileData, "smod");
            CreateModelIndicators(ReadModelFileData(abyFileData), ltstrShapes, "SHAPE");
            ltstrTrees = GetModelFileList(abyFileData, "smod");
            CreateModelIndicators(ReadModelFileData(abyFileData), ltstrTrees, "TREE");
            ltstrGrass = GetModelFileList(abyFileData, "smod");
            CreateModelIndicators(ReadModelFileData(abyFileData), ltstrGrass, "GRASS");
            ltstrAnimPrime = GetModelFileList(abyFileData, "vani");
            CreateModelIndicators(ReadModelFileData(abyFileData), ltstrAnimPrime, "ANIM_MAIN");
            ltstrAnimSec = GetModelFileList(abyFileData, "vani");
            CreateModelIndicators(ReadModelFileData(abyFileData), ltstrAnimSec, "ANIM_SEC");
            ltstrDungeons = GetModelFileList(abyFileData, "dg");
            CreateModelIndicators(ReadModelFileData(abyFileData), ltstrDungeons, "DUNG");

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
                                              (int.Parse(abyHeightMap[nZPos / 2 * nSideLen * 2 + nXPos / 2 * 2].ToString()) - 10000) / 50,
                                              nZPos));
                    ltv3Nrmls.Add(-Vector3.forward);

                    if (nXPos % 2 == 0 && nZPos % 2 == 0) {

                        nTxtrDataIndex++;
                    }

                    if (nXPos > 0 && nZPos > 0) {

                        nTxtrCheckIndex = int.Parse(abyTxtrData[nTxtrDataIndex + 1 - nSideLen + 1].ToString());

                        if (nZPos >= nMapSize - 1 || 
                            (nTxtrIndex != nTxtrCheckIndex &&
                             nTxtrCheckIndex < ltmtlGroundTxtrs.Count && 
                             nTxtrCheckIndex >= 0)) {

                            if (nTxtrCheckIndex < ltmtlGroundTxtrs.Count && nTxtrCheckIndex >= 0) {

                                nTxtrIndex = nTxtrCheckIndex;
                            }

                            ltmtlGroundMapList.Add(ltmtlGroundTxtrs[nTxtrIndex]);

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
                            }
                            else { 

                                nStartIndex = nZPos + 1;
                            }
                        }
                    }
                }
            }

            mrGroundGen.sharedMaterials = ltmtlGroundMapList.ToArray();

            Mesh mhGround = new Mesh();
            mhGround.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;

            mhGround.CombineMeshes(ltciMeshes.ToArray(), false);

            mfGroundInfo.sharedMesh = mhGround;
        }
        catch (Exception exError) {

            Debug.Log("Method: Start, Action: Loading file, Exception: " + exError.Message + ". Stacktrace: " + exError.StackTrace);
        }
        finally {

            if (fsAccess != null) {

                fsAccess.Close();
            }
        }
    }

    // Update is called once per frame
    void Update() {

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
        mhGround.Optimize();
        mfGroundInfo.mesh = mhGround;

        ciMeshSelect.mesh = mfGroundInfo.sharedMesh;
        ciMeshSelect.transform = mfGroundInfo.transform.localToWorldMatrix;

        return ciMeshSelect;
    }

    /// <summary>
    ///     LoadTextureDXT (Courtesy of user: jeff-smith, site: https://answers.unity.com, 
    ///                     page: https://answers.unity.com/questions/555984/can-you-load-dds-textures-during-runtime.html)
    /// </summary>
    Texture2D LoadTextureDXT(String strFilePathName) {

        FileStream fsAccess = new FileStream(strFilePathName, FileMode.Open, FileAccess.Read);

        byte[] ddsBytes = new byte[fsAccess.Length];
        fsAccess.Read(ddsBytes, 0, (int)fsAccess.Length);
        fsAccess.Close();

        byte ddsSizeCheck = ddsBytes[4];
        if (ddsSizeCheck != 124)
            throw new Exception("Invalid DDS DXTn texture. Unable to read");  //this header byte should be 124 for DDS image files

        int height = ddsBytes[13] * 256 + ddsBytes[12];
        int width = ddsBytes[17] * 256 + ddsBytes[16];

        int DDS_HEADER_SIZE = 128;
        byte[] dxtBytes = new byte[ddsBytes.Length - DDS_HEADER_SIZE];
        Buffer.BlockCopy(ddsBytes, DDS_HEADER_SIZE, dxtBytes, 0, ddsBytes.Length - DDS_HEADER_SIZE);

        Texture2D texture = new Texture2D(width, height, TextureFormat.DXT5, false);
        texture.LoadRawTextureData(dxtBytes);
        texture.Apply();

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

    List<string> GetModelFileList(byte[] abyFileData, string strFileType) {

        byte[] abyFileSectData = new byte[256];
        int nFileTypeLen = strFileType.Length;
        int nFileCount = BitConverter.ToInt32(abyFileData, nByteFilePos);
        string strFoundText = "";
        List<string> ltstrFileList = new List<string>();

        nByteFilePos += 4;

        for (int nCounter = 0; nCounter < nFileCount; nCounter++) {

            Buffer.BlockCopy(abyFileData, nByteFilePos, abyFileSectData, 0, 256);

            strFoundText = Encoding.UTF8.GetString(abyFileSectData);
            ltstrFileList.Add(strFoundText.Substring(0, strFoundText.IndexOf(".") + nFileTypeLen + 1));

            nByteFilePos += 256;
        }

        return ltstrFileList;
    }

    Dictionary<int, List<Vector3[]>> ReadModelFileData(byte[] abyFileData) {

        UInt32 nModelFileCounts = 0;
        int nFileIndex = 0;
        Dictionary<int, List<Vector3[]>> dictCoods = new Dictionary<int, List<Vector3[]>>();

        try {

            nModelFileCounts = BitConverter.ToUInt32(abyFileData, nByteFilePos);

            for (int nCounter = 0; nCounter < nModelFileCounts; nCounter++) {

                nFileIndex = (int)BitConverter.ToUInt32(abyFileData, nByteFilePos += 4);

                if (!dictCoods.ContainsKey(nFileIndex)) {

                    dictCoods.Add(nFileIndex, new List<Vector3[]>());
                }

                dictCoods[nFileIndex].Add(new Vector3[] {
                    new Vector3(BitConverter.ToSingle(abyFileData, nByteFilePos += 4),
                                BitConverter.ToSingle(abyFileData, nByteFilePos += 4),
                                BitConverter.ToSingle(abyFileData, nByteFilePos += 4)),
                    new Vector3(BitConverter.ToSingle(abyFileData, nByteFilePos += 4),
                                BitConverter.ToSingle(abyFileData, nByteFilePos += 4),
                                BitConverter.ToSingle(abyFileData, nByteFilePos += 4)),
                    new Vector3(BitConverter.ToSingle(abyFileData, nByteFilePos += 4),
                                BitConverter.ToSingle(abyFileData, nByteFilePos += 4),
                                BitConverter.ToSingle(abyFileData, nByteFilePos += 4))
                });
            }

            nByteFilePos += 4;
        }
        catch (Exception exError) {

            throw exError;
        }

        return dictCoods;
    }

    void CreateModelIndicators(Dictionary<int, List<Vector3[]>> dictModelInfo, 
                               List<string> ltstrNames,
                               string strType) {

        int nFileNum = 0;
        int nFileNameNum = 0;

        foreach (KeyValuePair<int, List<Vector3[]>> kvpModelData in dictModelInfo) {
                    
            foreach (Vector3[] av3Coords in kvpModelData.Value) { 
                
                PlaceModelIndicators(ltstrNames[kvpModelData.Key] + "-" + ++nFileNameNum + "-" + strType + ++nFileNum,
                                     av3Coords[0]);
            }

            nFileNameNum = 0;
        }
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
        goCloneHolder.AddComponent<MeshRenderer>().materials = goIndicator.GetComponent<MeshRenderer>().materials;
        tfCloneLoc.position = new Vector3(v3Pos.x, 0, v3Pos.z);
        tfCloneLoc.rotation = goIndicator.transform.rotation;
        tfCloneLoc.localScale = goIndicator.transform.localScale;
    }
}
