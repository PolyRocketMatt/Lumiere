using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public class Lumiere : MonoBehaviour
{
    [Header("General Settings")] 
    [SerializeField] private bool useGPU = true;
    [SerializeField] private ComputeShader shader;

    [Header("Skybox Settings")] 
    [SerializeField] private Texture skyboxTexture;
    
    private RenderTexture _target;
    private Camera _camera;
    private List<LumiereModel> _models;
    private Material _accumulateMaterial;
    
    private int _currentFrame;
    
    private static readonly int CurrentFrame = Shader.PropertyToID("_CurrentFrame");
    private static readonly int Result = Shader.PropertyToID("Result");
    private static readonly int CameraToWorld = Shader.PropertyToID("_CameraToWorld");
    private static readonly int CameraInverseProjection = Shader.PropertyToID("_CameraInverseProjection");
    private static readonly int SkyboxTexture = Shader.PropertyToID("_SkyboxTexture");
    private static readonly int PixelOffset = Shader.PropertyToID("_PixelOffset");

    private void Awake()
    {
        _camera = GetComponent<Camera>();
        _currentFrame = 0;
    }

    private void Start()
    {
        CollectModels();
    }

    private void Update()
    {
        if (transform.hasChanged)
        {
            _currentFrame = 0;
            transform.hasChanged = false;
        }
    }

    private void OnRenderImage(RenderTexture source, RenderTexture destination)
    {
        SetShaderParams();
        Render(destination);   
    }

    private void Render(RenderTexture destination)
    {
        InitRenderTexture();
        
        //  Set target and dispatch
        shader.SetTexture(0, Result, _target);
        var threadGroupsX = Mathf.CeilToInt(Screen.width / 8.0f);
        var threadGroupsY = Mathf.CeilToInt(Screen.height / 8.0f);
        shader.Dispatch(0, threadGroupsX, threadGroupsY, 1);
        
        //  Blit to screen
        _accumulateMaterial ??= new Material(Shader.Find("Hidden/AccumulateShader"));
        _accumulateMaterial.SetFloat(CurrentFrame, _currentFrame);
        Graphics.Blit(_target, destination, _accumulateMaterial);
        
        //  Update frame
        _currentFrame++;
    }

    private void InitRenderTexture()
    {
        if (_target != null && _target.width == Screen.width && _target.height == Screen.height) 
            return;
        if (_target != null)
            _target.Release();
        
        _target = new RenderTexture(Screen.width, Screen.height, 0, RenderTextureFormat.ARGBFloat, RenderTextureReadWrite.Linear)
        {
            enableRandomWrite = true
        };
        _target.Create();
    }

    private void SetShaderParams()
    {
        //  Camera matrices
        shader.SetMatrix(CameraToWorld, _camera.cameraToWorldMatrix);
        shader.SetMatrix(CameraInverseProjection, _camera.projectionMatrix.inverse);
        shader.SetVector(PixelOffset, new Vector2(Random.value, Random.value));
        
        //  Skybox
        shader.SetTexture(0, SkyboxTexture, skyboxTexture);
    }
    
    private void OnDisable()
    {
        _models?.Clear();
    }

    private void CollectModels()
    {
        _models ??= new List<LumiereModel>();
        _models.Clear();

        var objects = FindObjectsByType<MonoBehaviour>(FindObjectsSortMode.None).OfType<ILumiereObject>().ToArray();
        foreach (var lumiereObject in objects)
            _models.Add(lumiereObject.ToModel());
    }
    
}
