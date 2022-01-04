using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;

public class WLDFileLoader : MonoBehaviour {

    private MeshFilter mfGroundInfo;

    // Start is called before the first frame update
    void Start() {

        FileStream fsAccess = null;
        byte[] abyFileData;
        byte[] abyHeightMap;
        byte[] abyTxtrData;
        char[] acharFileChars;
        int nCharDataLen = 0;
        int nMapSize = 0;
        long lHeightMapLen = 0;
        long lTxtrDataLen = 0;
        Regex rgChecker;
        Material mtlCreate;
        List<Material> ltmtlGroundTxtrs = new List<Material>();
        string strFoundText = "";
        string strGroundTxtrFileNames = "";
        string strWaterTxtrFileName = "";
        string strSkyTxtrFileName = "";
        string strCloudTxtrFileNames = "";
        List<string> ltstrCloudTxtrs = new List<string>();
        int nCharPos = 0;
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
            abyFileData = new byte[fsAccess.Length];
            fsAccess.Read(abyFileData, 0, (int)fsAccess.Length);
            fsAccess.Close();

            nMapSize = (Convert.ToInt32(abyFileData[5]) << 8) | Convert.ToInt32(abyFileData[6]);
            nSideLen = (nMapSize / 2) + 1;
            lHeightMapLen = ((((nMapSize / 2) + 1) * ((nMapSize / 2) + 1)) * 2);
            lTxtrDataLen = (((nMapSize / 2) + 1) * ((nMapSize / 2) + 1));

            acharFileChars = Encoding.UTF8.GetString(abyFileData).ToCharArray();
            nCharDataLen = acharFileChars.Length;
            abyHeightMap = new byte[lHeightMapLen];
            abyTxtrData = new byte[lTxtrDataLen];

            Buffer.BlockCopy(abyFileData, 8, abyHeightMap, 0, (int)lHeightMapLen);
            Buffer.BlockCopy(abyFileData, (int)lHeightMapLen + 8, abyTxtrData, 0, (int)lTxtrDataLen);

            rgChecker = new Regex("[a-zA-Z0-9\\._]");

            while (nCharPos < nCharDataLen && !strFoundText.Contains(".wtr")) {
                
                if (rgChecker.IsMatch(acharFileChars[nCharPos].ToString())) {

                    strFoundText += acharFileChars[nCharPos].ToString();
                }
                else {

                    strFoundText = "";
                }

                if (strFoundText.Contains(".tga") || strFoundText.Contains(".dds")) {

                    if (strGroundTxtrFileNames != "") {

                        strGroundTxtrFileNames += ", ";
                    }

                    strGroundTxtrFileNames += strFoundText.Trim();

                    mtlCreate = new Material(Shader.Find("Standard"));
                    mtlCreate.mainTexture = LoadTextureDXT("Assets/Textures/Environment/" +
                                                           strFoundText.Trim().Replace(".tga", ".dds"));

                    ltmtlGroundTxtrs.Add(mtlCreate);
                    strFoundText = "";
                }

                nCharPos++;
            }

            ltmtlGroundTxtrs.Reverse();

            if (strFoundText.Contains(".wtr")) {

                strWaterTxtrFileName = strFoundText.Trim();
            }

            strFoundText = "";

            while (nCharPos < nCharDataLen && !strFoundText.Contains(".bmp")) {

                if (rgChecker.IsMatch(acharFileChars[nCharPos].ToString())) {

                    strFoundText += acharFileChars[nCharPos].ToString();
                }
                else {

                    strFoundText = "";
                }

                nCharPos++;
            }

            if (strFoundText.Contains(".bmp")) {

                strSkyTxtrFileName = strFoundText.Trim();
            }

            strFoundText = "";

            while (nCharPos < nCharDataLen) {

                if (rgChecker.IsMatch(acharFileChars[nCharPos].ToString())) {

                    strFoundText += acharFileChars[nCharPos].ToString();
                }
                else {

                    strFoundText = "";
                }

                if (strFoundText.Contains(".tga") || strFoundText.Contains(".dds")) {
                    
                    if (strCloudTxtrFileNames != "") {

                        strCloudTxtrFileNames += ", ";
                    }

                    strCloudTxtrFileNames += strFoundText.Trim();

                    ltstrCloudTxtrs.Add(strFoundText.Trim().Replace(".tga", ".dds"));
                    strFoundText = "";
                }

                nCharPos++;
            }

            Debug.Log("Map Size: " + nMapSize);
            Debug.Log("Ground Textures: " + String.Join(", ", strGroundTxtrFileNames));
            Debug.Log("Water Texture File: " + strWaterTxtrFileName);
            Debug.Log("Sky Texture: " + strSkyTxtrFileName);
            Debug.Log("Cloud Textures: " + strCloudTxtrFileNames);

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

            mfGroundInfo.mesh = mhGround;
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
}
