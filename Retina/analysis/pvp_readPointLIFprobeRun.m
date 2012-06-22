function [time,ret1,ret2,ret3,ret4,ret5,ret6] = pvp_readPointLIFprobeRun(runnumber,filename,inputs)

%Allows for the input of 1 to 6 variables and returns time and values of input variables
%Inputs must be a cell array of strings {'A','B',...'}
%Example: [time,G_I,V] = pvp_readPointLIFprobe1('ConeP5',{'G_I','V'})

filename = ["../output/Run",num2str(runnumber),"/pt",filename,".txt"]

myfile = fopen(filename,"r");
if myfile == -1
 error("No input file");
end %if

count = 0;
while ~feof(myfile)
lines = fgetl(myfile);
count = count + 1;
end %while

nLines = count;

fclose(myfile);

myfile = fopen(filename,"r");

%Creates an array of the input strings

s = inputs;

G_E  = zeros(nLines, 1);
G_I  = zeros(nLines, 1);
G_IB = zeros(nLines, 1);
V    = zeros(nLines, 1);
Vth  = zeros(nLines, 1);
A    = zeros(nLines, 1);
location = zeros(nLines, 1);


   for i_step = 1:nLines

          line_str = fgets(myfile);

     name_pos = index(line_str, ":");
     name_tmp = line_str(1:name_pos-1);
   

     location_begin_ndx = index(line_str, "k=");
     location_end_ndx = index(line_str, "G_E=");
     location_tmp_str = line_str(location_begin_ndx+2);
     location(i_step) = str2double(location_tmp_str);
   

     G_E_begin_ndx = index(line_str, "G_E=");
     G_E_end_ndx = index(line_str, "G_I=");
     G_E_tmp_str = line_str(G_E_begin_ndx+5:G_E_end_ndx-1);
     G_E(i_step) = str2double(G_E_tmp_str);
  

     G_I_begin_ndx = index(line_str, "G_I=");
     G_I_end_ndx = index(line_str, "G_IB=");
     G_I_tmp_str = line_str(G_I_begin_ndx+5:G_I_end_ndx-1);
     G_I(i_step) = str2double(G_I_tmp_str);
    

     G_IB_begin_ndx = index(line_str, "G_IB=");
     G_IB_end_ndx = index(line_str, "V=");
     G_IB_tmp_str = line_str(G_IB_begin_ndx+6:G_IB_end_ndx-1);
     G_IB(i_step) = str2double(G_IB_tmp_str);
    

     V_begin_ndx = index(line_str, "V=");
     V_end_ndx = index(line_str, "Vth=");
     V_tmp_str = line_str(V_begin_ndx+2:V_end_ndx-1);
     V(i_step) = str2double(V_tmp_str);
    

     Vth_begin_ndx = index(line_str, "Vth=");
     Vth_end_ndx = index(line_str, "a=");
     Vth_tmp_str = line_str(Vth_begin_ndx+4:Vth_end_ndx-2);
     Vth(i_step) = str2double(Vth_tmp_str);
   

     A_begin_ndx = index(line_str, "a=");
     A_end_ndx = length(line_str);
     A_tmp_str = line_str(A_begin_ndx+2:A_end_ndx-1);
     A(i_step) = str2double(A_tmp_str);
   end %for



%Output values for each input string


ret1 = [];
ret2 = [];
ret3 = [];
ret4 = [];
ret5 = [];
ret6 = [];

outp = zeros(nLines,length(s));

   for i=1:length(s)

	switch s{i}

		case 'A'

	          outp(:,i) = A;
                          

                case 'G_I'

                  outp(:,i) = G_I;

		case 'G_IB'

		  outp(:,i) = G_IB;

		case 'G_E'

		  outp(:,i) = G_E;

		case 'Vth'

		  outp(:,i) = Vth;

		case 'V'

		  outp(:,i) = V;

	   end %switch

    end %for

if length(s)>0
   ret1 = outp(:,1);
end %if

if length(s)>1
   ret2 = outp(:,2);
end %if

if length(s)>2
   ret3 = outp(:,3);
end %if

if length(s)>3
   ret4 = outp(:,4);
end %if

if length(s)>4
   ret5 = outp(:,5);
end %if

if length(s)>5;
   ret6 = outp(:,6);
end %if

time = nLines;

fclose(myfile);

