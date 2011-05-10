function [connID, connIndex] = pvp_connectionID()

  connIndex = struct;
  ij_conn = 0;
  global N_CONNECTIONS
  global SPIKING_FLAG
  global TRAINING_FLAG

  if ( SPIKING_FLAG == 1 )
    
  N_CONNECTIONS = 14;
  connID = cell(1,N_CONNECTIONS);


				% retinal connections
  ij_conn = ij_conn + 1;
  connIndex.r_lgn = ij_conn;
  connID{ 1, ij_conn } =  'image to retina';

  ij_conn = ij_conn + 1;
  connIndex.r_lgn = ij_conn;
  connID{ 1, ij_conn } =  'Retina to LGN';

  ij_conn = ij_conn + 1;
  connIndex.r_lgninhff = ij_conn;
  connID{ 1, ij_conn } =  'Retina to LGNInhFF';

				% LGN connections
  %ij_conn = ij_conn + 1;
  %connIndex.lgn_lgninhff = ij_conn;
  %connID{ 1, ij_conn } =  'LGN to LGNInhFF';

  ij_conn = ij_conn + 1;
  connIndex.lgn_lgninh = ij_conn;
  connID{ 1, ij_conn } =  'LGN to LGNInh';

  ij_conn = ij_conn + 1;
  connIndex.lgn_l1 = ij_conn;
  connID{ 1, ij_conn } =  'LGN to L1';

  ij_conn = ij_conn + 1;
  connIndex.lgn_l1inhff = ij_conn;
  connID{ 1, ij_conn } =  'LGN to L1InhFF';

				% LGNInhFF connections
  ij_conn = ij_conn + 1;
  connIndex.lgninhff_lgn = ij_conn;
  connID{ 1, ij_conn } =  'LGNInhFF to LGN';

  %ij_conn = ij_conn + 1;
  %connIndex.lgninhff_lgn_inhB = ij_conn;
  %connID{ 1, ij_conn } =  'LGNInhFF to LGN InhB';

  %ij_conn = ij_conn + 1;
  %connIndex.lgninhff_lgninhff_exc = ij_conn;
  %connID{ 1, ij_conn } =  'LGNInh to LGNInhFF Exc';

  %ij_conn = ij_conn + 1;
  %connIndex.lgninhff_lgninhff = ij_conn;
  %connID{ 1, ij_conn } =  'LGNInhFF to LGNInhFF';

  %ij_conn = ij_conn + 1;
  %connIndex.lgninhff_lgninhff_inhB = ij_conn;
  %connID{ 1, ij_conn } =  'LGNInhFF to LGNInhFF InhB';

  %ij_conn = ij_conn + 1;
  %connIndex.lgninhff_lgninh = ij_conn;
  %connID{ 1, ij_conn } =  'LGNInhFF to LGNInh';

  %ij_conn = ij_conn + 1;
  %connIndex.lgninhff_lgninh_inhB = ij_conn;
  %connID{ 1, ij_conn } =  'LGNInhFF to LGNInh InhB';

				% LGNInh connections
  ij_conn = ij_conn + 1;
  connIndex.lgninh_lgn = ij_conn;
  connID{ 1, ij_conn } =  'LGN Inh to LGN';

  %ij_conn = ij_conn + 1;
  %connIndex.lgninh_lgninh_exc = ij_conn;
  %connID{ 1, ij_conn } =  'LGN Inh to LGN Inh Exc';

  %ij_conn = ij_conn + 1;
  %connIndex.lgninh_lgninh = ij_conn;
  %connID{ 1, ij_conn } =  'LGN Inh to LGN Inh';


				% V1 connections
  ij_conn = ij_conn + 1;
  connIndex.l1_lgn = ij_conn;
  connID{ 1, ij_conn } =  'L1 to LGN';

  ij_conn = ij_conn + 1;
  connIndex.l1_lgninh = ij_conn;
  connID{ 1, ij_conn } =  'L1 to LGN Inh';

  ij_conn = ij_conn + 1;
  connIndex.l1_l1 = ij_conn;
  connID{ 1, ij_conn } =  'L1 to L1';

  ij_conn = ij_conn + 1;
  connIndex.l1_l1inh = ij_conn;
  connID{ 1, ij_conn } =  'L1 to L1 Inh';


				% V1 Inh FF connections
  ij_conn = ij_conn + 1;
  connIndex.l1inhff_l1 = ij_conn;
  connID{ 1, ij_conn } =  'L1InhFF to L1';

  %ij_conn = ij_conn + 1;
  %connIndex.l1inhff_l1_inhB = ij_conn;
  %connID{ 1, ij_conn } =  'L1InhFF to L1 InhB';

  %ij_conn = ij_conn + 1;
  %connIndex.l1inhff_l1inhff_exc = ij_conn;
  %connID{ 1, ij_conn } =  'L1InhFF to L1InhFF Exc';

  %ij_conn = ij_conn + 1;
  %connIndex.l1inhff_l1inhff = ij_conn;
  %connID{ 1, ij_conn } =  'L1InhFF to L1InhFF';

  %ij_conn = ij_conn + 1;
  %connIndex.l1inhff_l1ihff_inhB = ij_conn;
  %connID{ 1, ij_conn } =  'L1InhFF to L1InhFF InhB';

  ij_conn = ij_conn + 1;
  connIndex.l1inhff_l1inh = ij_conn;
  connID{ 1, ij_conn } =  'L1InhFF to L1Inh';

  %ij_conn = ij_conn + 1;
  %connIndex.l1inhff_l1ih_inhB = ij_conn;
  %connID{ 1, ij_conn } =  'L1InhFF to L1Inh InhB';


				% V1 Inh connections
  ij_conn = ij_conn + 1;
  connIndex.l1inh_l1 = ij_conn;
  connID{ 1, ij_conn } =  'L1Inh to L1';

  %ij_conn = ij_conn + 1;
  %connIndex.l1inh_l1inhff = ij_conn;
  %connID{ 1, ij_conn } =  'L1Inh to L1Inh FF';

  ij_conn = ij_conn + 1;
  connIndex.l1inh_l1inh = ij_conn;
  connID{ 1, ij_conn } =  'L1Inh to L1Inh';

  %ij_conn = ij_conn + 1;
  %connIndex.l1inh_l1inh_exc = ij_conn;
  %connID{ 1, ij_conn } =  'L1Inh to L1Inh Exc';

  N_CONNECTIONS = 16;


else % NON_SPIKING
  
  N_CONNECTIONS = 6;
  connID = cell(1,N_CONNECTIONS);

  ij_conn = ij_conn + 1;
  connIndex.r_l1 = ij_conn;
  connID{ 1, ij_conn } =  'Image2R';
  
  ij_conn = ij_conn + 1;
  connIndex.r_l1 = ij_conn;
  connID{ 1, ij_conn } =  'R2L1';
  
  ij_conn = ij_conn + 1;
  connIndex.r_l1inh = ij_conn;
  connID{ 1, ij_conn } =  'R2L1Inh';
  
  ij_conn = ij_conn + 1;
  connIndex.l1_l1_geisler = ij_conn;
  connID{ 1, ij_conn } =  'L1ToL1G';
  
  ij_conn = ij_conn + 1;
  connIndex.l1_l1_geisler_target = ij_conn;
  connID{ 1, ij_conn } =  'L1ToL1GT';
  
  ij_conn = ij_conn + 1;
  connIndex.l1_l1_geisler_distractor = ij_conn;
  connID{ 1, ij_conn } =  'L1ToL1GD';
  
    
    N_CONNECTIONS = N_CONNECTIONS + 3;
    connID = [connID, cell(1,3)];
    
    ij_conn = ij_conn + 1;
    connIndex.l1_geisler_l1_geisler2 = ij_conn;
    connID{ 1, ij_conn } =  'L1GToL1G2';
    
    ij_conn = ij_conn + 1;
    connIndex.l1_l1_geisler_target = ij_conn;
    connID{ 1, ij_conn } =  'L1GToL1G2T';
    
    ij_conn = ij_conn + 1;
    connIndex.l1_l1_geisler_distractor = ij_conn;
    connID{ 1, ij_conn } =  'L1GToL1G2D';
          
      N_CONNECTIONS = N_CONNECTIONS + 3;
      connID = [connID, cell(1,3)];
      
      ij_conn = ij_conn + 1;
      connIndex.l1_geisler2_l1_geisler3 = ij_conn;
      connID{ 1, ij_conn } =  'L1G2ToL1G3';
      
      ij_conn = ij_conn + 1;
      connIndex.l1_l1_geisler_target = ij_conn;
      connID{ 1, ij_conn } =  'L1G2ToL1G3T';
      
      ij_conn = ij_conn + 1;
      connIndex.l1_l1_geisler_distractor = ij_conn;
      connID{ 1, ij_conn } =  'L1G2ToL1G3D';
      
	
      N_CONNECTIONS = N_CONNECTIONS + 3;
      connID = [connID, cell(1,3)];
      
      ij_conn = ij_conn + 1;
      connIndex.l1_geisler3_l1_geisler4 = ij_conn;
      connID{ 1, ij_conn } =  'L1G3ToL1G4';
      
      ij_conn = ij_conn + 1;
      connIndex.l1_l1_geisler_target = ij_conn;
      connID{ 1, ij_conn } =  'L1G3ToL1G4T';
      
      ij_conn = ij_conn + 1;
      connIndex.l1_l1_geisler_distractor = ij_conn;
      connID{ 1, ij_conn } =  'L1G3ToL1G4D';
      
	
	connID = [connID, cell(1,1)];
	
	ij_conn = ij_conn + 1;
	connIndex.l1_geisler4_l1_geisler4 = ij_conn;
	connID{ 1, ij_conn } =  'L1G4ToL1G4';
	
  
 if TRAINING_FLAG == -1
    
  N_CONNECTIONS = 6;
  connIndex.l1_geisler_l1_geisler = 7;
  connID{ 1, 7 } =  'L1GToL1G';
   
 elseif TRAINING_FLAG == -2

    N_CONNECTIONS = 9;
  connIndex.l1_geisler2_l1_geisler2 = 10;
  connID{ 1, 10 } =  'L1G2ToL1G2';
   

 elseif TRAINING_FLAG == -3

    N_CONNECTIONS = 12;
  connIndex.l1_geisler3_l1_geisler3 = 13;
  connID{ 1, 13 } =  'L1G3ToL1G3';
   

 end%%if
 
end%%if % spiking_flag