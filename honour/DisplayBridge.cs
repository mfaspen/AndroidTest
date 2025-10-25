using System;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.Rendering;
using static UnityEngine.GraphicsBuffer;

public class DisplayBridge : MonoBehaviour
{
    private AndroidJavaObject unityActivity;
    private AndroidJavaClass pluginBridge;

    public static int[] textureID = new int[3];

    public static int[] setTextureID = new int[3];


    protected RenderTexture Texture1;
    IntPtr TextureID1;
    protected RenderTexture Texture2;
    IntPtr TextureID2;
    protected RenderTexture Texture3;
    IntPtr TextureID3;
    protected RenderTexture Texture4;
    IntPtr TextureID4;


    [DllImport("SecondRendering")]
    private static extern void setPointer(int[] id);

    [DllImport("SecondRendering")] // 这里填你自己的so名字
    private static extern System.IntPtr GetRenderEventFunc();

    Camera camera;
    int width = 256;
    int height = 256;
    protected void CreateTexture()
    {

        //var desc = new RenderTextureDescriptor(256, 256, RenderTextureFormat.ARGB32, 24)
        //{
        //    //msaaSamples = Mathf.Max(QualitySettings.antiAliasing, 2),
        //    sRGB = (QualitySettings.activeColorSpace == ColorSpace.Linear),
        //    useMipMap = false,
        //    autoGenerateMips = false
        //};
        var desc = new RenderTextureDescriptor(width, height, RenderTextureFormat.ARGB32, 24)
        {
            msaaSamples = 1,                // 非 MSAA
            sRGB = false,                   // 非 sRGB
            useMipMap = false,
            autoGenerateMips = false
        };
        //Texture1.Create();
        //Texture2.Create();
        //Texture3.Create();
        //Texture4.Create();

        Texture1 = new RenderTexture(desc);
        Texture2 = new RenderTexture(desc);
        Texture3= new RenderTexture(desc);
        Texture4 =new RenderTexture(desc);

        //Texture1.Create();
        //Texture2.Create();
        //Texture3.Create();
        //Texture4.Create();

        //RenderTexture src1 = RenderTexture.active;
        //RenderTexture.active = Texture1;
        //GL.Clear(false, true, Color.red);
        //RenderTexture.active = Texture2;
        //GL.Clear(false, true, Color.red);
        //RenderTexture.active = Texture3;
        //GL.Clear(false, true, Color.red);
        //RenderTexture.active = Texture4;
        //GL.Clear(false, true, Color.red);
        //RenderTexture.active = src1;

        TextureID1 = Texture1.GetNativeTexturePtr();
        TextureID2 = Texture2.GetNativeTexturePtr();
        TextureID3 = Texture3.GetNativeTexturePtr();
        TextureID4 = Texture4.GetNativeTexturePtr();




    }


    // Start is called before the first frame update
    void Start()
    {

        camera = GetComponent<Camera>();

        //Debug.Log("START DB Called PluginBridge.startPresentation()");

        CreateTexture();

        // 1️⃣ 拿到 Unity 的当前 Activity
        using (AndroidJavaClass unityPlayer = new AndroidJavaClass("com.unity3d.player.UnityPlayer"))
        {
            unityActivity = unityPlayer.GetStatic<AndroidJavaObject>("currentActivity");

        }


        // 2️⃣ 找到我们 Kotlin 侧的 PluginBridge 单例类
        pluginBridge = new AndroidJavaClass("com.example.Application.PluginBridge");


        setPointer(setTextureID);


        // 3️⃣ 调用 Kotlin 的 startPresentation(activity)
        pluginBridge.CallStatic("startPresentation", unityActivity);


        //GL.IssuePluginEvent(GetRenderEventFunc(), 1);


       Debug.Log("looking Start" );


    }
    RenderTexture TagTex;
    int cID = 0;
    public void Update()
    {
        Debug.Log("looking Update");
        switch (cID % 4)
        {
            case 0:
                camera.targetTexture = Texture1;
                textureID[0] = (int)Texture1.GetNativeTexturePtr();
                break;
            case 1:
                camera.targetTexture = Texture2;
                textureID[0] = (int)Texture2.GetNativeTexturePtr();
                break;
            case 2:
                camera.targetTexture = Texture3;
                textureID[0] = (int)Texture3.GetNativeTexturePtr();
                break;
            case 3:
                camera.targetTexture = Texture4;
                textureID[0] = (int)Texture4.GetNativeTexturePtr();
                break;
        }
        cID++;
        //Graphics.Blit(Texture1, Texture2);
        Debug.Log("looking OnEndCameraRendering: " + (uint)textureID[0]+ " cID: "+ cID+ "TagTex："+ Texture1.GetNativeTexturePtr()+ "camera.targetTexture:"+ camera.targetTexture.GetNativeTexturePtr());
        Debug.Log("looking Graphics API: " + SystemInfo.graphicsDeviceType);

        var tex = Texture1;
        Debug.Log($"RT id: {tex.GetNativeTexturePtr()} format: {tex.format} colorSpace: {QualitySettings.activeColorSpace}");


    }
    CommandBuffer cb;

    public void OnPostRender()
    {
        //RenderTexture tmp = RenderTexture.GetTemporary(256, 256, 24, RenderTextureFormat.ARGB32, RenderTextureReadWrite.Linear);
        //Graphics.Blit(camera.targetTexture, tmp);
        setTextureID[0] = (int)camera.targetTexture.GetNativeTexturePtr();

        Camera.main.AddCommandBuffer(CameraEvent.AfterEverything, cb);
        //if(cID ==1)
        //{
        GL.IssuePluginEvent(GetRenderEventFunc(), 1);
        //}

    }



    void OnEnable()
    {
        RenderPipelineManager.endCameraRendering += OnEndCameraRendering;
    }

    void OnDisable()
    {
        RenderPipelineManager.endCameraRendering -= OnEndCameraRendering;
    }

    void OnEndCameraRendering(ScriptableRenderContext context, Camera camera)
    {
        setTextureID[0] = textureID[0];

        Debug.Log("DB OnEndCameraRendering:" + (uint)setTextureID[0]);
        // ✅ 等价于 OnPostRender()

    }


    void OnApplicationPause(bool paused)
    {
        if (pluginBridge == null) return;

        if (paused)
        {
            pluginBridge.CallStatic("onPause");
        }
        else
        {
            pluginBridge.CallStatic("onResume");
        }
    }

    void OnDestroy()
    {
        if (pluginBridge != null)
        {
            pluginBridge.CallStatic("stopPresentation");
        }
    }
}