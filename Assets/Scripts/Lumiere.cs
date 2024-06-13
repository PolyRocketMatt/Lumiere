using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public class Lumiere : MonoBehaviour
{
    [Header("General Settings")] 
    [SerializeField] private ComputeShader shader;
    
    private RenderTexture _target;
    private List<LumiereModel> _models;
    
    private static readonly int Result = Shader.PropertyToID("Result");

    private void Start()
    {
        CollectModels();
    }

    private void OnRenderImage(RenderTexture source, RenderTexture destination)
    {
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
        Graphics.Blit(_target, destination);
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
    
    private void OnDisable()
    {
        _models?.Clear();
    }

    private void CollectModels()
    {
        _models ??= new List<LumiereModel>();
        _models.Clear();

        var objects = FindObjectsOfType<MonoBehaviour>().OfType<ILumiereObject>().ToArray();
        foreach (var lumiereObject in objects)
            _models.Add(lumiereObject.ToModel());
    }
    
}
