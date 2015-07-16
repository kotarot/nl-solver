/*******************************************************/
/** 「空白マス非存在性」に基づきフラグセットする関数  **/
/*******************************************************/

#include "./main.hpp"
#include "./fixflag.hpp"


extern Board* board;


void generateFixFlag(){

	bool complete_flag = false;
	while(!complete_flag){
	complete_flag = true; // フラグオン
	
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			//cout << x << "," << y << endl;
			Box* trgt_box = board->box(x,y);
			if(trgt_box->isTypeAllFixed()) continue;
			Direction d = {true,true,true,true};
			
			if(x==0){
				d.w = false;
			}
			else{
				Box* find_box = board->box(x-1,y);
				if(find_box->isTypeAllFixed() && !find_box->isEastLineFixed()){
					d.w = false;
				}
				if(trgt_box->isTypeNumber() && find_box->isTypeNumber()){
					d.w = false;
				}
			}
			if(x==(board->getSizeX()-1)){
				d.e = false;
			}
			else{
				Box* find_box = board->box(x+1,y);
				if(find_box->isTypeAllFixed() && !find_box->isWestLineFixed()){
					d.e = false;
				}
				if(trgt_box->isTypeNumber() && find_box->isTypeNumber()){
					d.e = false;
				}
			}
			if(y==0){
				d.n = false;
			}
			else{
				Box* find_box = board->box(x,y-1);
				if(find_box->isTypeAllFixed() && !find_box->isSouthLineFixed()){
					d.n = false;
				}
				if(trgt_box->isTypeNumber() && find_box->isTypeNumber()){
					d.n = false;
				}
			}
			if(y==(board->getSizeY()-1)){
				d.s = false;
			}
			else{
				Box* find_box = board->box(x,y+1);
				if(find_box->isTypeAllFixed() && !find_box->isNorthLineFixed()){
					d.s = false;
				}
				if(trgt_box->isTypeNumber() && find_box->isTypeNumber()){
					d.s = false;
				}
			}
			
			// 半固定マスに対する操作
			int half_line = -1;
			if(trgt_box->isTypeHalfFixed()){
				// 北にだけ線を持つ
				if(trgt_box->isNorthLineFixed()){
					half_line = NORTH;
					Box* find_box = board->box(x,y-1);
					if(find_box->isEastLineFixed()){
						d.e = false;
					}
					if(find_box->isWestLineFixed()){
						d.w = false;
					}
					
					// 西マス対象
					if(x!=0){
						find_box = board->box(x-1,y);
						if(find_box->isTypeHalfFixed() && find_box->isNorthLineFixed()){
							d.w = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.w = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x-1,y);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.w = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.e = false; d.s = false;
							}
						}
					}
					// 東マス対象
					if(x!=(board->getSizeX()-1)){
						find_box = board->box(x+1,y);
						if(find_box->isTypeHalfFixed() && find_box->isNorthLineFixed()){
							d.e = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.e = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x+1,y);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.e = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.w = false; d.s = false;
							}
						}
					}
					// 南マス対象
					if(y!=(board->getSizeY()-1)){
						find_box = board->box(x,y+1);
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.s = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x,y+1);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.s = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.e = false; d.w = false;
							}
						}
					}
				}
				// 東だけに線を持つ
				if(trgt_box->isEastLineFixed()){
					half_line = EAST;
					Box* find_box = board->box(x+1,y);
					if(find_box->isNorthLineFixed()){
						d.n = false;
					}
					if(find_box->isSouthLineFixed()){
						d.s = false;
					}
					
					// 北マス対象
					if(y!=0){
						find_box = board->box(x,y-1);
						if(find_box->isTypeHalfFixed() && find_box->isEastLineFixed()){
							d.n = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.n = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x,y-1);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.n = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.s = false; d.w = false;
							}
						}
					}
					// 南マス対象
					if(y!=(board->getSizeY()-1)){
						find_box = board->box(x,y+1);
						if(find_box->isTypeHalfFixed() && find_box->isEastLineFixed()){
							d.s = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.s = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x,y+1);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.s = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.n = false; d.w = false;
							}
						}
					}
					// 西マス対象
					if(x!=0){
						find_box = board->box(x-1,y);
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.w = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x-1,y);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.w = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.n = false; d.s = false;
							}
						}
					}
				}
				// 南だけに線を持つ
				if(trgt_box->isSouthLineFixed()){
					half_line = SOUTH;
					Box* find_box = board->box(x,y+1);
					if(find_box->isEastLineFixed()){
						d.e = false;
					}
					if(find_box->isWestLineFixed()){
						d.w = false;
					}
					
					// 西マス対象
					if(x!=0){
						find_box = board->box(x-1,y);
						if(find_box->isTypeHalfFixed() && find_box->isSouthLineFixed()){
							d.w = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.w = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x-1,y);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.w = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.n = false; d.e = false;
							}
						}
					}
					// 東マス対象
					if(x!=(board->getSizeX()-1)){
						find_box = board->box(x+1,y);
						if(find_box->isTypeHalfFixed() && find_box->isSouthLineFixed()){
							d.e = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.e = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x+1,y);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.e = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.n = false; d.w = false;
							}
						}
					}
					// 北マス対象
					if(y!=0){
						find_box = board->box(x,y-1);
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.n = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x,y-1);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.n = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.e = false; d.w = false;
							}
						}
					}
				}
				// 西だけに線を持つ
				if(trgt_box->isWestLineFixed()){
					half_line = WEST;
					Box* find_box = board->box(x-1,y);
					if(find_box->isNorthLineFixed()){
						d.n = false;
					}
					if(find_box->isSouthLineFixed()){
						d.s = false;
					}
					
					// 北マス対象
					if(y!=0){
						find_box = board->box(x,y-1);
						if(find_box->isTypeHalfFixed() && find_box->isWestLineFixed()){
							d.n = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.n = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x,y-1);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.n = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.e = false; d.s = false;
							}
						}
					}
					// 南マス対象
					if(y!=(board->getSizeY()-1)){
						find_box = board->box(x,y+1);
						if(find_box->isTypeHalfFixed() && find_box->isWestLineFixed()){
							d.s = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.s = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x,y+1);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.s = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.n = false; d.e = false;
							}
						}
					}
					// 東マス対象
					if(x!=(board->getSizeX()-1)){
						find_box = board->box(x+1,y);
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.e = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x+1,y);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.e = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.n = false; d.s = false;
							}
						}
					}
				}
			}
			
			// 数字マスに対する操作
			if(trgt_box->isTypeNumber()){
				// 西マス対象
				if(x!=0){
					Box* find_box = board->box(x-1,y);
					if(find_box->isTypeHalfFixed()){
						int find_num = getConnectedNumber(x-1,y);
						if(find_num>0 && trgt_box->getNumber()!=find_num){
							d.w = false;
						}
						if(find_num>0 && trgt_box->getNumber()==find_num){
							d.n = false; d.e = false; d.s = false;
						}
					}
				}
				// 東マス対象
				if(x!=(board->getSizeX()-1)){
					Box* find_box = board->box(x+1,y);
					if(find_box->isTypeHalfFixed()){
						int find_num = getConnectedNumber(x+1,y);
						if(find_num>0 && trgt_box->getNumber()!=find_num){
							d.e = false;
						}
						if(find_num>0 && trgt_box->getNumber()==find_num){
							d.n = false; d.s = false; d.w = false;
						}
					}
				}
				// 北マス対象
				if(y!=0){
					Box* find_box = board->box(x,y-1);
					if(find_box->isTypeHalfFixed()){
						int find_num = getConnectedNumber(x,y-1);
						if(find_num>0 && trgt_box->getNumber()!=find_num){
							d.n = false;
						}
						if(find_num>0 && trgt_box->getNumber()==find_num){
							d.e = false; d.s = false; d.w = false;
						}
					}
				}
				// 南マス対象
				if(y!=(board->getSizeY()-1)){
					Box* find_box = board->box(x,y+1);
					if(find_box->isTypeHalfFixed()){
						int find_num = getConnectedNumber(x,y+1);
						if(find_num>0 && trgt_box->getNumber()!=find_num){
							d.s = false;
						}
						if(find_num>0 && trgt_box->getNumber()==find_num){
							d.n = false; d.e = false; d.w = false;
						}
					}
				}
			}
			
			
			int count = 0;
			if(d.n) count++;
			if(d.e) count++;
			if(d.s) count++;
			if(d.w) count++;
			
			if(trgt_box->isTypeBlank() && count==2){
				complete_flag = false;
				trgt_box->setTypeAllFixed();
				if(d.n && half_line!=NORTH){
					trgt_box->fixNorthLine();
					Box* find_box = board->box(x,y-1);
					if(find_box->isTypeNumber()){
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else{
							find_box->setTypeAllFixed();
							find_box->fixSouthLine();
						}
					}
					else{
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else if(find_box->isTypeHalfFixed()){
							find_box->setTypeAllFixed();
							find_box->fixSouthLine();
						}
						else{
							find_box->setTypeHalfFixed();
							find_box->fixSouthLine();
						}
					}
				}
				if(d.e && half_line!=EAST){
					trgt_box->fixEastLine();
					Box* find_box = board->box(x+1,y);
					if(find_box->isTypeNumber()){
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else{
							find_box->setTypeAllFixed();
							find_box->fixWestLine();
						}
					}
					else{
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else if(find_box->isTypeHalfFixed()){
							find_box->setTypeAllFixed();
							find_box->fixWestLine();
						}
						else{
							find_box->setTypeHalfFixed();
							find_box->fixWestLine();
						}
					}
				}
				if(d.s && half_line!=SOUTH){
					trgt_box->fixSouthLine();
					Box* find_box = board->box(x,y+1);
					if(find_box->isTypeNumber()){
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else{
							find_box->setTypeAllFixed();
							find_box->fixNorthLine();
						}
					}
					else{
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else if(find_box->isTypeHalfFixed()){
							find_box->setTypeAllFixed();
							find_box->fixNorthLine();
						}
						else{
							find_box->setTypeHalfFixed();
							find_box->fixNorthLine();
						}
					}
				}
				if(d.w && half_line!=WEST){
					trgt_box->fixWestLine();
					Box* find_box = board->box(x-1,y);
					if(find_box->isTypeNumber()){
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else{
							find_box->setTypeAllFixed();
							find_box->fixEastLine();
						}
					}
					else{
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else if(find_box->isTypeHalfFixed()){
							find_box->setTypeAllFixed();
							find_box->fixEastLine();
						}
						else{
							find_box->setTypeHalfFixed();
							find_box->fixEastLine();
						}
					}
				}
			}
			if(trgt_box->isTypeNumber() && count==1){
				complete_flag = false;
				trgt_box->setTypeAllFixed();
				if(d.n){
					trgt_box->fixNorthLine();
					Box* find_box = board->box(x,y-1);
					if(find_box->isTypeAllFixed()){
						cout << "Conflict!!" << endl;
						exit(1);
					}
					else if(find_box->isTypeHalfFixed()){
						find_box->setTypeAllFixed();
						find_box->fixSouthLine();
					}
					else{
						find_box->setTypeHalfFixed();
						find_box->fixSouthLine();
					}
				}
				if(d.e){
					trgt_box->fixEastLine();
					Box* find_box = board->box(x+1,y);
					if(find_box->isTypeAllFixed()){
						cout << "Conflict!!" << endl;
						exit(1);
					}
					else if(find_box->isTypeHalfFixed()){
						find_box->setTypeAllFixed();
						find_box->fixWestLine();
					}
					else{
						find_box->setTypeHalfFixed();
						find_box->fixWestLine();
					}
				}
				if(d.s){
					trgt_box->fixSouthLine();
					Box* find_box = board->box(x,y+1);
					if(find_box->isTypeAllFixed()){
						cout << "Conflict!!" << endl;
						exit(1);
					}
					else if(find_box->isTypeHalfFixed()){
						find_box->setTypeAllFixed();
						find_box->fixNorthLine();
					}
					else{
						find_box->setTypeHalfFixed();
						find_box->fixNorthLine();
					}
				}
				if(d.w){
					trgt_box->fixWestLine();
					Box* find_box = board->box(x-1,y);
					if(find_box->isTypeAllFixed()){
						cout << "Conflict!!" << endl;
						exit(1);
					}
					else if(find_box->isTypeHalfFixed()){
						find_box->setTypeAllFixed();
						find_box->fixEastLine();
					}
					else{
						find_box->setTypeHalfFixed();
						find_box->fixEastLine();
					}
				}
			}
		}
	}
	
	}
	
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			Box* trgt_box = board->box(x,y);
			if(trgt_box->isTypeAllFixed()) cout << "2";
			else if(trgt_box->isTypeHalfFixed()) cout << "1";
			else cout << "0";
		}
		cout << endl;
	}
	
	// for(int y=0;y<board->getSizeY();y++){
		// for(int x=0;x<board->getSizeX();x++){
			// Box* trgt_box = board->box(x,y);
			// if(trgt_box->isNorthLineFixed()){
				// cout << " N";
			// }
			// else{
				// cout << " +";
			// }
			// if(trgt_box->isEastLineFixed()){
				// cout << " E";
			// }
			// else{
				// cout << " +";
			// }
			// cout << " ";
		// }
		// cout << endl;
		// for(int x=0;x<board->getSizeX();x++){
			// Box* trgt_box = board->box(x,y);
			// if(trgt_box->isWestLineFixed()){
				// cout << " W";
			// }
			// else{
				// cout << " +";
			// }
			// if(trgt_box->isSouthLineFixed()){
				// cout << " S";
			// }
			// else{
				// cout << " +";
			// }
			// cout << " ";
		// }
		// cout << endl;
	// }
}
