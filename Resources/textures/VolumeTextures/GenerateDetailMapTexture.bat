ffmpeg -i "./DetailMapSequence/DetailMap%%04d.tif" -filter_complex scale=128:128,tile=128x1 DetailMapStrip.tiff

nvdxt -file DetailMapStrip.tiff -outdir ../../../Run/textures -quality_highest -a8 -volumeMap -nomipmap -output CloudDetailMap3D.dds
pause