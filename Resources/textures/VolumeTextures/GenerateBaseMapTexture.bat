ffmpeg -i "./BaseMapSequence/BaseMap%%04d.tif" -filter_complex scale=128:128,tile=128x1 BaseMapStrip.tiff

nvdxt -file BaseMapStrip.tiff -outdir ../../../Run/textures -quality_highest -a8 -volumeMap -nomipmap -output CloudBaseShapes3D.dds
pause