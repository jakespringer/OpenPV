object_name = {"031"; "032"; "033"; "034"; "035"; "036"; "037"; "038"; "039"; "040"; "041"; "042"; "043"; "044"; "045"; "046"; "047"; "048"; "049"; "050"};
for i_object = 1 : 0 %%length(object_name)
padChips([], ...
	 object_name{i_object}, ...
	       [], ...
	       [], ...
	       [], ...
	       [], ...
	       [], ...
	       [], ...
	       [])
endfor

for i_object = 1 : length(object_name)
		 chipFileOfFilenames([], ...
				     object_name{i_object}, ...
				     [], ...
				     [], ...
				     [], ...
				     [], ...
				     [], ...
				     [], ...
				     []);
endfor
