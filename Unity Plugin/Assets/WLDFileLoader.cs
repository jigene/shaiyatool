using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;

public class WLDFileLoader : MonoBehaviour
{
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
        List<string> ltstrGroundTxtrs = new List<string>();
        string strFoundText = "";
        string strWaterTxtrFileName = "";
        string strSkyTxtrFileName = "";
        List<string> ltstrCloudTxtrs = new List<string>();
        int nCharPos = 0;

        try
        {
            fsAccess = new FileStream("Assets/Model/0.wld", FileMode.Open, FileAccess.Read);
            abyFileData = new byte[fsAccess.Length];
            fsAccess.Read(abyFileData, 0, (int)fsAccess.Length);

            nMapSize = (Convert.ToInt32(abyFileData[5]) << 8) | Convert.ToInt32(abyFileData[6]);
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

                if (strFoundText.Contains(".tga")) {

                    ltstrGroundTxtrs.Add(strFoundText.Trim());
                    strFoundText = "";
                }

                nCharPos++;
            }

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

            int nSideLen = (nMapSize / 2) + 1;
            Debug.Log("Map Side Length: " + nSideLen);
            int x = 0;
            int y = 11;
            int nDataIndex = y / 2 * nSideLen * 2 + x / 2 * 2;
            int nCurrVertex = 0;
            int nSection = nMapSize / 16;
            int nZPos = 0;
            List<Vector3> ltv3Verts = new List<Vector3>();
            List<int> ltnTriages = new List<int>();
            List<Vector3> ltv3Nrmls = new List<Vector3>();
            List<Vector2> ltv2UV = new List<Vector2>();
        //    Debug.Log(nDataIndex);
        //    Debug.Log(abyHeightMap[nDataIndex]);
        //    Debug.Log((int.Parse(abyHeightMap[nDataIndex].ToString()) - 10000) / 50);

            Debug.Log("Map Size: " + nMapSize);
            Debug.Log("Ground Textures: " + String.Join(", ", ltstrGroundTxtrs));
            Debug.Log("Water Texture File: " + strWaterTxtrFileName);
            Debug.Log("Sky Texture: " + strSkyTxtrFileName);
            Debug.Log("Cloud Textures: " + String.Join(", ", ltstrCloudTxtrs));

            MeshRenderer mrGroundGen = gameObject.AddComponent<MeshRenderer>();
            MeshFilter mfGroundInfo = gameObject.AddComponent<MeshFilter>();
            Mesh mhGround = new Mesh();

            mrGroundGen.sharedMaterial = new Material(Shader.Find("Standard"));

            for (int nXPos = 0; nXPos < nMapSize; nXPos++) {

                while (nZPos < nMapSize)  {
                    ltv3Verts.Add(new Vector3(nXPos,
                                              (int.Parse(abyHeightMap[nZPos / 2 * nSideLen * 2 + nXPos / 2 * 2].ToString()) - 10000) / 50,
                                              nZPos));
                    ltv3Verts.Add(new Vector3(nXPos + 1,
                                              (int.Parse(abyHeightMap[nZPos / 2 * nSideLen * 2 + (nXPos + 1) / 2 * 2].ToString()) - 10000) / 50,
                                              nZPos));
                    ltv3Verts.Add(new Vector3(nXPos,
                                              (int.Parse(abyHeightMap[(nZPos + 1) / 2 * nSideLen * 2 + nXPos / 2 * 2].ToString()) - 10000) / 50,
                                              nZPos + 1));
                    ltv3Verts.Add(new Vector3(nXPos + 1,
                                              (int.Parse(abyHeightMap[(nZPos + 1) / 2 * nSideLen * 2 + (nXPos + 1) / 2 * 2].ToString()) - 10000) / 50,
                                              nZPos + 1));

                    ltnTriages.Add(nCurrVertex);
                    ltnTriages.Add(nCurrVertex + 2);
                    ltnTriages.Add(nCurrVertex + 1);
                    ltnTriages.Add(nCurrVertex + 2);
                    ltnTriages.Add(nCurrVertex + 3);
                    ltnTriages.Add(nCurrVertex + 1);

                    ltv3Nrmls.Add(-Vector3.forward);
                    ltv3Nrmls.Add(-Vector3.forward);
                    ltv3Nrmls.Add(-Vector3.forward);
                    ltv3Nrmls.Add(-Vector3.forward);

                    nZPos++;
                    nCurrVertex++;
                }

                nZPos = 0;
                    //ltv2UV.Add(new Vector2(nXPos, (int.Parse(abyHeightMap[nZPos / 2 * nSideLen * 2 + nXPos / 2 * 2].ToString()) - 10000) / 50));
                    //ltv2UV.Add(new Vector2(nXPos + 1, (int.Parse(abyHeightMap[nZPos / 2 * nSideLen * 2 + nXPos / 2 * 2].ToString()) - 10000) / 50));
                    //ltv2UV.Add(new Vector2(nXPos, ((int.Parse(abyHeightMap[nZPos / 2 * nSideLen * 2 + nXPos / 2 * 2].ToString()) - 10000) / 50) + 1));
                    //ltv2UV.Add(new Vector2(nXPos + 1, ((int.Parse(abyHeightMap[nZPos / 2 * nSideLen * 2 + nXPos / 2 * 2].ToString()) - 10000) / 50) + 1));
            }

            mhGround.vertices = ltv3Verts.ToArray();
            mhGround.triangles = ltnTriages.ToArray();
            mhGround.normals = ltv3Nrmls.ToArray();
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
}
