using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;

public class WLDFileLoader : MonoBehaviour
{
    private MeshFilter mfGroundInfo;

    // Start is called before the first frame update
    void Start()
    {
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
        List<Material> ltmtlGroundTxtrs = new List<Material>();
        string strFoundText = "";
        string stGroundTxtrFileNames = "";
        string strWaterTxtrFileName = "";
        string strSkyTxtrFileName = "";
        List<string> ltstrCloudTxtrs = new List<string>();
        int nCharPos = 0;
        int nSideLen = 0;
        int nCurrVertIndex = 0;
        List<Vector3> ltv3Verts = new List<Vector3>();
        List<int> ltnTriages = new List<int>();
        List<Vector3> ltv3Nrmls = new List<Vector3>();
        List<CombineInstance> ltciMeshes = new List<CombineInstance>();
        List<Vector2> ltv2UV = new List<Vector2>();

        MeshRenderer mrGroundGen = gameObject.AddComponent<MeshRenderer>();
        Mesh mhGround;

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

                    if (stGroundTxtrFileNames != "") {

                        stGroundTxtrFileNames += ", ";
                    }

                    stGroundTxtrFileNames += strFoundText.Trim();

                    ltmtlGroundTxtrs.Add(Resources.Load("Assets/Textures/Environment/"  + 
                                                        strFoundText.Trim().Replace(".tga", ".dds")) as Material);
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

                if (strFoundText.Contains(".tga")) {

                    ltstrCloudTxtrs.Add(strFoundText.Trim());
                    strFoundText = "";
                }

                nCharPos++;
            }

            //    int x = 0;
            //    int y = 11;
            //    int nDataIndex = y / 2 * nSideLen * 2 + x / 2 * 2;
            //    Debug.Log(nDataIndex);
            //    Debug.Log(abyHeightMap[nDataIndex]);
            //    Debug.Log((int.Parse(abyHeightMap[nDataIndex].ToString()) - 10000) / 50);

            Debug.Log("Map Size: " + nMapSize);
            Debug.Log("Ground Textures: " + String.Join(", ", stGroundTxtrFileNames));
            Debug.Log("Water Texture File: " + strWaterTxtrFileName);
            Debug.Log("Sky Texture: " + strSkyTxtrFileName);
            Debug.Log("Cloud Textures: " + String.Join(", ", ltstrCloudTxtrs));

            mfGroundInfo = gameObject.AddComponent<MeshFilter>();
            mrGroundGen.sharedMaterial = new Material(Shader.Find("Standard"));
            //            mrGroundGen.materials = ltmtlGroundTxtrs.ToArray();

     //       int textNum = -1;

            for (int nXPos = 0; nXPos < nMapSize; nXPos++) {

                for (int nZPos = 0; nZPos < nMapSize; nZPos++) {
                    ltv3Verts.Add(new Vector3(nXPos,
                                              (int.Parse(abyHeightMap[nZPos / 2 * nSideLen * 2 + nXPos / 2 * 2].ToString()) - 10000) / 50,
                                              nZPos));
                    ltv3Nrmls.Add(-Vector3.forward);

 //                   if (textNum >= 255 && nZPos == 0) {

//                        textNum = -1;
 //                   }

  //                  if (textNum < 255 && textNum != int.Parse(abyTxtrData[nZPos * nSideLen + nXPos].ToString())) {

    //                    Debug.Log("X: " + nXPos + ", Z: " + nZPos + ", ID: " + abyTxtrData[nZPos * nSideLen + nXPos].ToString());
        //                textNum = int.Parse(abyTxtrData[nZPos * nSideLen + nXPos].ToString());
      //              }

                    if (nXPos > 0 && nZPos > 0) {

                        nCurrVertIndex = ltv3Verts.Count - 1;

                        ltnTriages.Add(nCurrVertIndex - 1);
                        ltnTriages.Add((nCurrVertIndex - nMapSize) - 1);
                        ltnTriages.Add(nCurrVertIndex - nMapSize);
                        ltnTriages.Add(nCurrVertIndex - nMapSize);
                        ltnTriages.Add(nCurrVertIndex);
                        ltnTriages.Add(nCurrVertIndex - 1);

                        if (nZPos >= nMapSize - 1) {

                            ltciMeshes.Add(BuildGroundLane(ltv3Verts, ltnTriages, ltv3Nrmls, ltv2UV));

                            ltv3Verts.RemoveRange(0, nMapSize);
                            ltv3Nrmls.RemoveRange(0, nMapSize);
                            ltnTriages.Clear();
//                            ltv2UV.Clear();
                        }
                    }
                }
            }

            if (ltv3Verts.Count > 0) { 

                ltciMeshes.Add(BuildGroundLane(ltv3Verts, ltnTriages, ltv3Nrmls, ltv2UV));
            }

            mhGround = new Mesh();
            mhGround.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;
            mhGround.CombineMeshes(ltciMeshes.ToArray());
            mfGroundInfo.mesh = mhGround;
        }
        catch (Exception exError) {

            Debug.Log("Method: Start, Action: Loading file, Exception: " + exError.Message);
        }
        finally {

            if (fsAccess != null) {

                fsAccess.Close();
            }
        }
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    CombineInstance BuildGroundLane(List<Vector3> ltv3Verts,
                                    List<int> ltnTriages,
                                    List<Vector3> ltv3Nrmls,
                                    List<Vector2> ltv2UV) { 

        Mesh mhGround = new Mesh();
        CombineInstance ciMeshSelect = new CombineInstance();

        mhGround.vertices = ltv3Verts.ToArray();
        mhGround.triangles = ltnTriages.ToArray();
        mhGround.normals = ltv3Nrmls.ToArray();
//        mhGround.uv = ltv2UV.ToArray();
        mfGroundInfo.mesh = mhGround;

        ciMeshSelect.mesh = mfGroundInfo.sharedMesh;
        ciMeshSelect.transform = mfGroundInfo.transform.localToWorldMatrix;

        return ciMeshSelect;
    }
}
