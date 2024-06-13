// ReSharper disable once CheckNamespace
public interface IGeometry { }

public interface IGeometryConvertible<out T> where T : IGeometry
{
    T GetGeometry();
}