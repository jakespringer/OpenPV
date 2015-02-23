disp('======= KITTI DevKit Demo =======');
clear all; close all; dbstop error;

addpath('devkit/matlab/')
addpath('~/workspace/PetaVision/mlab/util')

% error threshold
tau = 3;

outdir = '/nh/compneuro/Data/Depth/LCA/benchmark/train/slp_nf_512_ICA/';
%timestamp = [outdir '/timestamps/DepthImage.txt'];
outPvpFile = [outdir 'a6_SLP_Recon.pvp'];
gtPvpFile = [outdir 'a2_DepthDownsample.pvp'];
scoreDir = [outdir 'scores/']
imageDir = '/nh/compneuro/Data/Depth/stereo_flow/multiview/training/image_2/'

mkdir(scoreDir);

[data_est, hdr_est] = readpvpfile(outPvpFile);
[data_gt, hdr_gt] = readpvpfile(gtPvpFile);

numFrames = hdr_est.nbands;
errList = zeros(1, numFrames);

for(i = 1:numFrames)
   estData = data_est{i}.values' * 256;
   gtData = data_gt{i}.values' * 256;

   handle = figure;
   targetTime = data_est{i}.time;
   imageFilename = [imageDir sprintf('%06d_10.png', targetTime)];
   outFilename = [scoreDir num2str(targetTime) '_EstVsImage.png']
   im = imread(imageFilename);
   [nx, ny, nf] = size(estData);
   im = imresize(im, [nx, ny]);
   subplot(2, 1, 1);
   imshow(disp_to_color(estData));
   subplot(2, 1, 2);
   imshow(im);
   %handle = imshow([disp_to_color(estData); im]);
   saveas(handle, outFilename);

   d_err = disp_error(gtData,estData,tau);
   errList(i) = d_err;

   %Mask out estdata with mask
   estData(find(gtData == 0)) = 0;
   outFilename = [scoreDir num2str(targetTime) '_gtVsEst.png']

   figure;
   handle = imshow(disp_to_color([estData;gtData]));
   title(sprintf('Error: %.2f %%',d_err*100));
   saveas(handle, outFilename);
end

aveFile = fopen([scoreDir 'aveError.txt'], 'w');
fprintf(aveFile, '%f', mean(errList(:)));
fclose(aveFile);
