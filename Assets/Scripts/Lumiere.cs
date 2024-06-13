using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public class Lumiere : MonoBehaviour
{
    [Header("General Settings")] 

    private List<LumiereModel> _models;

    private void Start()
    {
        CollectModels();
    }

    private void OnRenderImage(RenderTexture source, RenderTexture destination)
    {
        throw new NotImplementedException();
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
