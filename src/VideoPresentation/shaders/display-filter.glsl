vec3 displayFilterPixel(vec2 uv)
{
    if (Parameters.LedMatrix.x > 0.0) return ledMatrixPixel(uv);
    if (Parameters.VfdStructure.x > 0.0) return vfdPixel(uv);
    if (Parameters.DotMatrixGeometry.x > 0.0) return dotMatrixPixel(uv);
    if (Parameters.NativeSegmentLevels.y > Parameters.NativeSegmentLevels.x) return nativeSegmentPixel(uv);
    if (Parameters.RasterSegmentGeometry.x > 0.0) return rasterSegmentPixel(uv);
    if (Parameters.EPaperInkAndColor.x > 0.0) return ePaperPixel(uv);
    if (Parameters.ProjectionGeometry.x > 0.0) return projectionPixel(uv);
    if (Parameters.PlasmaGeometry.x > 0.0) return plasmaPixel(uv);
    if (Parameters.VectorSource.x > 0.0) return vectorRasterPixel(uv);
    if (Parameters.FixedGrid.x > 0.0) return fixedPixel(uv);
    return crtPixel(uv);
}
