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

    private MeshFilter mfGroundInfo;
    private MeshRenderer mrGroundGen;
    private int nByteFilePos = 0;
    private Dictionary<string, Dictionary<string, Vector3[]>> dictModelCoords = new Dictionary<string, Dictionary<string, Vector3[]>>();
    private Dictionary<string, List<string>> dictModelNamesByType = new Dictionary<string, List<string>>();
    private string strFileSelPath = "";
    private long lHeightMapLen = 0;
    private long lTxtrDataLen = 0;
    private int nMapSize = 0;
    private int nSideLen = 0;
    private byte[] abyHeightMap;

    // Start is called before the first frame update
    void Start() {

        mrGroundGen = gameObject.AddComponent<MeshRenderer>();
        mfGroundInfo = gameObject.AddComponent<MeshFilter>();
    }

    public void GenerateGround(string strFilePath) { 
    
        FileStream fsAccess = null;
        byte[] abyFileData;
        byte[] abyTxtrData;
        byte[] abyFileSectData;
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

        try {

            strFileSelPath = strFilePath;

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
            dictModelCoords.Add("Buildings", CreateModelIndicators(ReadModelFileData(abyFileData), ltstrBuildings, "BLG"));

            ltstrShapes = GetModelFileList(abyFileData, "smod");
            dictModelCoords.Add("Shapes", CreateModelIndicators(ReadModelFileData(abyFileData), ltstrShapes, "SHAPE"));

            ltstrTrees = GetModelFileList(abyFileData, "smod");
            dictModelCoords.Add("Trees", CreateModelIndicators(ReadModelFileData(abyFileData), ltstrTrees, "TREE"));

            ltstrGrass = GetModelFileList(abyFileData, "smod");
            dictModelCoords.Add("Grass", CreateModelIndicators(ReadModelFileData(abyFileData), ltstrGrass, "GRASS"));

            ltstrAnimPrime = GetModelFileList(abyFileData, "vani");
            dictModelCoords.Add("Animations - Primary", CreateModelIndicators(ReadModelFileData(abyFileData), ltstrAnimPrime, "ANIM_MAIN"));

            ltstrAnimSec = GetModelFileList(abyFileData, "vani");
            dictModelCoords.Add("Animations - Secondary", CreateModelIndicators(ReadModelFileData(abyFileData), ltstrAnimSec, "ANIM_SEC"));

            ltstrDungeons = GetModelFileList(abyFileData, "dg");
            dictModelCoords.Add("Dungeons", CreateModelIndicators(ReadModelFileData(abyFileData), ltstrDungeons, "DUNG"));

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

            Debug.Log("Method: GenerateGround, Action: Loading file, Exception: " + exError.Message + ". Stacktrace: " + exError.StackTrace);
        }
        finally {

            if (fsAccess != null) {

                fsAccess.Close();
            }
        }
    }

    public void SaveFile() {

//        Vector3[] av3Vertices;
        FileStream fsAccess = null;
        byte[] abyFileData;
        byte[] abyTxtrData;
        byte[] abyFileSectData;
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

        try {

            if (strFileSelPath != "") {

                Vector3[] av3Vertices = mfGroundInfo.sharedMesh.vertices;

                for (int nXPos = 0; nXPos < nMapSize; nXPos++) {

                    for (int nZPos = 0; nZPos < nMapSize; nZPos++) {

                        abyHeightMap[nZPos / 2 * nSideLen * 2 + nXPos / 2 * 2] = Convert.ToByte(Math.Floor(((av3Vertices[nXPos + nZPos].y - 100) * 50) + 10000));
                    } 
                }



                //                    fsAccess = new FileStream(strFileSelPath, FileMode.Open, FileAccess.Read);
                //                    abyFileData = new byte[(int)fsAccess.Length];
                //                   fsAccess.Read(abyFileData, 0, (int)fsAccess.Length);
                //                   fsAccess.Close();

                //    fsAccess = new FileStream(strFileSelPath, FileMode.OpenOrCreate, FileAccess.Write);

                //    fsAccess.Close();

            }
            else {

                Debug.Log("No file was loaded for save.");
            }
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

    Dictionary<string, Vector3[]> CreateModelIndicators(Dictionary<int, List<Vector3[]>> dictModelInfo, 
                                                        List<string> ltstrNames,
                                                        string strType) {

        string strModelName = "";
        int nFileNum = 0;
        int nFileNameNum = 0;
        Dictionary<string, Vector3[]> dictModelCoords = new Dictionary<string, Vector3[]>();

        foreach (KeyValuePair<int, List<Vector3[]>> kvpModelData in dictModelInfo) {
                    
            foreach (Vector3[] av3Coords in kvpModelData.Value) {

                strModelName = ltstrNames[kvpModelData.Key] + "-" + ++nFileNameNum + "-" + strType + ++nFileNum;

                PlaceModelIndicators(strModelName, av3Coords[0]);

                dictModelCoords.Add(strModelName, av3Coords);
            }

            nFileNameNum = 0;
        }

        return dictModelCoords;
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

        foreach (KeyValuePair<string, Dictionary<string, Vector3[]>>  kvpModelData in dictModelCoords) {

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

            dictDistList.Add(strNewModelName, av3Coords);
            PlaceModelIndicators(strNewModelName, av3Coords[0]);
            goNew = GameObject.Find(strNewModelName);
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

        if (dictDistList != null) {

            string strNewModelName = strOrgModelName + "-" + ++nModelCount + "-ADDED" + (dictDistList.Count + 1);

            dictDistList.Add(strNewModelName, av3Coords);
            PlaceModelIndicators(strNewModelName, av3Coords[0]);
            goNew = GameObject.Find(strNewModelName);
        }

        return goNew;
    }

    public void UpdateModelCoordsByName(string strName, Vector3[] av3Coords) {

        Vector3[] v3Ret = new Vector3[] { new Vector3(), new Vector3(), new Vector3() };
        bool boolNotFound = true;

        foreach (Dictionary<string, Vector3[]> dictCoords in dictModelCoords.Values) { 

            if (dictCoords.ContainsKey(strName)) { 

                dictCoords[strName] = av3Coords;
                GameObject.Find(strName).transform.position = new Vector3(av3Coords[0].x, 0, av3Coords[0].z);
                boolNotFound = false;
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
    }

    public void RemoveModelIndicators() {

        foreach (Dictionary<string, Vector3[]> dictModelList in dictModelCoords.Values) {

            foreach (string strModelName in dictModelList.Keys) {

                RemoveModelIndicatorsByName(strModelName);
            }
        }
    }

    public void DeleteModelCoordsByName(string strName) {

        bool boolNotFound = true;

        foreach (Dictionary<string, Vector3[]> dictCoords in dictModelCoords.Values) { 

            if (dictCoords.ContainsKey(strName)) {

                dictCoords.Remove(strName);
                boolNotFound = false;
                break;
            }
        }

        if (boolNotFound) {

            Debug.Log("Model, '" + strName + "', not found for deletion.");
        }
    }

    public void ShowModelIndicators() {

        RemoveModelIndicators();

        foreach (Dictionary<string, Vector3[]> dictCoords in dictModelCoords.Values) {

            foreach (KeyValuePair<string, Vector3[]> kvpModelInfo in dictCoords) {

                PlaceModelIndicators(kvpModelInfo.Key, kvpModelInfo.Value[0]);
            }
        }
    }

    public void ShowModelIndicatorsByType(string strModelType) {

        RemoveModelIndicators();

        foreach (KeyValuePair<string, Dictionary<string, Vector3[]>> kvpModelData in dictModelCoords) {

            if (strModelType == kvpModelData.Key) { 

                foreach (KeyValuePair<string, Vector3[]> kvpModelInfo in kvpModelData.Value) {

                    PlaceModelIndicators(kvpModelInfo.Key, kvpModelInfo.Value[0]);
                }

                break;
            }
        }
    }
}

#if UNITY_EDITOR_WIN
public class WLDFileUpdater : EditorWindow {

    private Vector3[] av3ModelSel = new Vector3[] { new Vector3(), new Vector3(), new Vector3() };
    private Vector2 v2MainSelPos = new Vector2(),
                    v2FileSelPos = new Vector2(),
                    v2AddModelSelPos = new Vector2(),
                    v2SelModelSelPos = new Vector2(),
                    v2NewModelTypeSelPos = new Vector2();
    private GameObject goSelected = null;
    private string strCreateModelName = "";

    [MenuItem("Plugins/WLD File Updater")]
    static void Init() {

        EditorWindow.GetWindow(typeof(WLDFileUpdater), false, "WLD File Updater").Show();
    }

    void OnGUI() {
    
        WLDFileLoader wdlLoader = GameObject.Find("Ground Generator").GetComponent<WLDFileLoader>();
        string[] astrModelFileNames = Directory.GetFiles("Assets/Model/");
        string strWldFilePath = "";
        GUIStyle gsLabel = GUI.skin.GetStyle("Label");
        GUILayoutOption gloLabelHeight = GUILayout.Height(25),
                        gloSelHeight = GUILayout.Height(75);

        gsLabel.alignment = TextAnchor.MiddleLeft;
        
        v2MainSelPos = GUILayout.BeginScrollView(v2MainSelPos);
        GUILayout.BeginVertical("box");
        GUILayout.Label("Select WLD File:", gsLabel, gloLabelHeight);
        v2FileSelPos = GUILayout.BeginScrollView(v2FileSelPos, gloSelHeight);

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
        
        if (GUILayout.Button("UPDATE") && goSelected != null) {

            wdlLoader.UpdateModelCoordsByName(goSelected.name, av3ModelSel);
        }
        
        if (GUILayout.Button("DELETE") && 
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
                av3ModelSel[2] = new Vector3(0 , 1, 0);
                Selection.activeGameObject = wdlLoader.AddModelCoordsByName(strModelName, av3ModelSel);
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
        GUILayout.EndScrollView();
    }

     void OnSelectionChange() {

        Repaint();
     }
}
#endif