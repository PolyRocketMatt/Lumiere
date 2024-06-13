// ReSharper disable once CheckNamespace
public class LumiereModel
{
    private readonly IGeometry _geometry;
    private readonly IMaterial _material;

    public LumiereModel(IGeometry geometry, IMaterial material)
    {
        _geometry = geometry;
        _material = material;
    }
    
    public IGeometry Geometry => _geometry;
    public IMaterial Material => _material;
    
}