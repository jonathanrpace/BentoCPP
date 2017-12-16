nvdxt -file Lava_Albedo.tga -outdir   ../../Run/textures/ -quality_highest -dxt5   -timestamp -sharpenMethod SharpenMedium
nvdxt -file Lava_Material.tiff -outdir ../../Run/textures/ -quality_highest -dxt5 -timestamp -nmips 6
nvdxt -file Lava_Normal.tga -outdir   ../../Run/textures/ -quality_highest -dxt5nm -timestamp

nvdxt -file LavaLat_Albedo.tga -outdir   ../../Run/textures/ -quality_highest -dxt5   -timestamp -sharpenMethod SharpenMedium
nvdxt -file LavaLat_Material.tga -outdir ../../Run/textures/ -quality_highest -dxt5   -timestamp -sharpenMethod SharpenMedium
nvdxt -file LavaLat_Normal.tga -outdir   ../../Run/textures/ -quality_highest -dxt5nm -timestamp

nvdxt -file LavaLong_Albedo.tga -outdir   ../../Run/textures/ -quality_highest -dxt5   -timestamp -sharpenMethod SharpenMedium
nvdxt -file LavaLong_Material.tga -outdir ../../Run/textures/ -quality_highest -dxt5   -timestamp -sharpenMethod SharpenMedium
nvdxt -file LavaLong_Normal.tga -outdir   ../../Run/textures/ -quality_highest -dxt5nm -timestamp

nvdxt -file SmokeParticle.tga -outdir   ../../Run/textures/ -quality_highest -dxt5 -timestamp

nvdxt -file DirtyLens.tga -outdir   ../../Run/textures/ -quality_highest -u888 -timestamp

nvdxt -file AlbedoFluidGradient.png -outdir   ../../Run/textures/ -quality_highest -dxt3 -timestamp