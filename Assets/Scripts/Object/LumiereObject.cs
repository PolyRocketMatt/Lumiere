using UnityEngine;

// ReSharper disable once CheckNamespace
public abstract class LumiereObject<T> : MonoBehaviour, ILumiereObject, IGeometryConvertible<T> where T : IGeometry
{

    public abstract T GetGeometry();

    protected abstract IMaterial GetMaterial();

    public LumiereModel ToModel() => new(GetGeometry(), GetMaterial());

}