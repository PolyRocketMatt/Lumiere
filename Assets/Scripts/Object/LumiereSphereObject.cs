using UnityEngine;

// ReSharper disable once CheckNamespace
public class LumiereSphereObject : LumiereObject<SphereGeometry>
{
    private IMaterial _material;
    
    private void Start()
    {
        var components = GetComponents<MonoBehaviour>();
        foreach (var component in components)
        {
            //  Currently just set the last material found. Give an error or precedence in the future.
            if (component is IMaterial material)
                _material = material;
        }

        //  If no material was found, initialize a simple material.
        _material ??= gameObject.AddComponent<SimpleMaterial>();
    }

    public override SphereGeometry GetGeometry()
    {
        return new SphereGeometry
        {
            Origin = transform.position,
            Radius = transform.localScale.x / 2
        };
    }

    protected override IMaterial GetMaterial() => _material;
}