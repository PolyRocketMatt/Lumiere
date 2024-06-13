using UnityEngine;

// ReSharper disable once CheckNamespace
[System.Serializable]
public class SimpleMaterial : MonoBehaviour, IMaterial
{
    [Header("Simple Material Settings")]
    [SerializeField] private Color albedo = Color.white;
    
    public Color Albedo => albedo;
}