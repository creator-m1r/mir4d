# Grid rendering fix

The grid renderer uses column-major matrices and camera-relative vertices. The grid pass must:

- multiply projection × rotation-only view in column-major order;
- never double-apply the camera translation;
- offset generated grid coordinates from the snapped anchor exactly once;
- keep the grid visible when the camera is close to the active grid plane;
- compute screen-space width and view direction from camera-relative segment positions.
