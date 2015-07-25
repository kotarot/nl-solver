/*******************************************************/
/** 「空白マス非存在性」に基づきフラグセットする関数  **/
/*******************************************************/

#include "./main.hpp"
#include "./fixflag.hpp"


extern Board* board;


////////////////////////////////////////////////////////////////
// 空白マスに対して「全固定」または「半固定」属性を与える
//
// 全固定:
//   空白マスで線がつながる2ポートが決定している
//     または
//   数字マスで線がつながる1ポートが決定している
//
// 半固定:
//   空白マスの1ポートが決定している
void generateFixFlag(){

	// 隣接数字を全固定にする
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			Box* trgt_box = board->box(x,y);
			if(trgt_box->isTypeNumber() && !trgt_box->isTypeAllFixed()){
				int trgt_num = trgt_box->getNumber();

				// 左端でなければ左隣を調べる
				if(x!=0){
					Box* find_box = board->box(x-1,y);
					// 同じ数字なら接続確定
					if(find_box->isTypeNumber() && find_box->getNumber() == trgt_num){
						trgt_box->setTypeAllFixed();
						trgt_box->fixWestLine();
						find_box->setTypeAllFixed();
						find_box->fixEastLine();
					}
				}

				// 右端でなければ右隣を調べる必要はない
				// (左隣を調べる処理に含まれるため)

				// 上端でなければ上隣を調べる
				if(y!=0){
					Box* find_box = board->box(x,y-1);
					// 同じ数字なら接続確定
					if(find_box->isTypeNumber() && find_box->getNumber() == trgt_num){
						trgt_box->setTypeAllFixed();
						trgt_box->fixNorthLine();
						find_box->setTypeAllFixed();
						find_box->fixSouthLine();
					}
				}

				// 下端でなければ下隣を調べる必要はない
				// (上隣を調べる処理に含まれるため)

			}
		}
	}

	bool complete_flag = false;
	while(!complete_flag){
	complete_flag = true; // フラグオン
	
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			//cout << x << "," << y << endl;
			Box* trgt_box = board->box(x,y);
			if(trgt_box->isTypeAllFixed()) continue;
			
			// 接続可能な隣接マス？
			Direction d = {true,true,true,true};
			
			// 左端
			if(x==0){
				d.w = false;
			}
			// 左端以外
			else{
				// 1個左のマスを調べる
				Box* find_box = board->box(x-1,y);
				if(find_box->isTypeAllFixed() && !find_box->isEastLineFixed()){
					d.w = false;
				}
				else if(trgt_box->isTypeNumber() && find_box->isTypeNumber()){
					d.w = false;
				}
			}
			
			// 右端
			if(x==(board->getSizeX()-1)){
				d.e = false;
			}
			// 右端以外
			else{
				// 1個右のマスを調べる
				Box* find_box = board->box(x+1,y);
				if(find_box->isTypeAllFixed() && !find_box->isWestLineFixed()){
					d.e = false;
				}
				else if(trgt_box->isTypeNumber() && find_box->isTypeNumber()){
					d.e = false;
				}
			}
			
			// 上端
			if(y==0){
				d.n = false;
			}
			// 上端以外
			else{
				// 1個上のマスを調べる
				Box* find_box = board->box(x,y-1);
				if(find_box->isTypeAllFixed() && !find_box->isSouthLineFixed()){
					d.n = false;
				}
				else if(trgt_box->isTypeNumber() && find_box->isTypeNumber()){
					d.n = false;
				}
			}
			
			// 下端
			if(y==(board->getSizeY()-1)){
				d.s = false;
			}
			// 下端
			else{
				// 1個下のマスを調べる
				Box* find_box = board->box(x,y+1);
				if(find_box->isTypeAllFixed() && !find_box->isNorthLineFixed()){
					d.s = false;
				}
				else if(trgt_box->isTypeNumber() && find_box->isTypeNumber()){
					d.s = false;
				}
			}
			
			// 半固定マスに対する操作
			int half_line = -1;
			if(trgt_box->isTypeHalfFixed()){

				// 北にだけ固定線を持つ
				if(trgt_box->isNorthLineFixed()){
					// 1個上のマスが東 (西) に固定線を持てば、このマスも東 (西) に固定線は持たない
					// ※迂回を考えない
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
						else if(find_box->isTypeNumber()){
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
							else if(trgt_num>0 && find_num>0 && trgt_num==find_num){
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
						else if(find_box->isTypeNumber()){
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
							else if(trgt_num>0 && find_num>0 && trgt_num==find_num){
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
							else if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.e = false; d.w = false;
							}
						}
					}
				}

				// 東だけに固定線を持つ
				else if(trgt_box->isEastLineFixed()){
					// 1個右のマスが北 (南) に固定線を持てば、このマスも北 (南) に固定線は持たない
					// ※迂回を考えない
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
						else if(find_box->isTypeNumber()){
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
							else if(trgt_num>0 && find_num>0 && trgt_num==find_num){
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
						else if(find_box->isTypeNumber()){
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
							else if(trgt_num>0 && find_num>0 && trgt_num==find_num){
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
							else if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.n = false; d.s = false;
							}
						}
					}
				}

				// 南だけに固定線を持つ
				else if(trgt_box->isSouthLineFixed()){
					// 1個下のマスが東 (西) に固定線を持てば、このマスも東 (西) に固定線は持たない
					// ※迂回を考えない
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
						else if(find_box->isTypeNumber()){
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
							else if(trgt_num>0 && find_num>0 && trgt_num==find_num){
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
						else if(find_box->isTypeNumber()){
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
							else if(trgt_num>0 && find_num>0 && trgt_num==find_num){
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
							else if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.e = false; d.w = false;
							}
						}
					}
				}

				// 西だけに固定線を持つ
				else if(trgt_box->isWestLineFixed()){
					// 1個左のマスが北 (南) に固定線を持てば、このマスも北 (南) に固定線は持たない
					// ※迂回を考えない
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
						else if(find_box->isTypeNumber()){
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
							else if(trgt_num>0 && find_num>0 && trgt_num==find_num){
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
						else if(find_box->isTypeNumber()){
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
							else if(trgt_num>0 && find_num>0 && trgt_num==find_num){
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
							else if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.n = false; d.s = false;
							}
						}
					}
				}
				
				// 半固定マスなのにどの報告にも固定線を持たない
				else {
					assert(!"No fixed line to any direction");
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
						else if(find_num>0 && trgt_box->getNumber()==find_num){
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
						else if(find_num>0 && trgt_box->getNumber()==find_num){
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
						else if(find_num>0 && trgt_box->getNumber()==find_num){
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
						else if(find_num>0 && trgt_box->getNumber()==find_num){
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
							cout << "Conflict!! (error: 50)" << endl;
							exit(50);
						}
						else{
							find_box->setTypeAllFixed();
							find_box->fixSouthLine();
						}
					}
					else{
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!  (error: 51)" << endl;
							exit(51);
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
							cout << "Conflict!! (error: 52)" << endl;
							exit(52);
						}
						else{
							find_box->setTypeAllFixed();
							find_box->fixWestLine();
						}
					}
					else{
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!! (error: 53)" << endl;
							exit(53);
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
							cout << "Conflict!! (error: 54)" << endl;
							exit(54);
						}
						else{
							find_box->setTypeAllFixed();
							find_box->fixNorthLine();
						}
					}
					else{
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!! (error: 55)" << endl;
							exit(55);
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
							cout << "Conflict!! (error: 56)" << endl;
							exit(56);
						}
						else{
							find_box->setTypeAllFixed();
							find_box->fixEastLine();
						}
					}
					else{
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!! (error: 57)" << endl;
							exit(57);
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
			else if(trgt_box->isTypeNumber() && count==1){
				complete_flag = false;
				trgt_box->setTypeAllFixed();
				if(d.n){
					trgt_box->fixNorthLine();
					Box* find_box = board->box(x,y-1);
					if(find_box->isTypeAllFixed()){
						cout << "Conflict!! (error: 60)" << endl;
						exit(60);
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
				else if(d.e){
					trgt_box->fixEastLine();
					Box* find_box = board->box(x+1,y);
					if(find_box->isTypeAllFixed()){
						cout << "Conflict!! (error: 61)" << endl;
						exit(61);
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
				else if(d.s){
					trgt_box->fixSouthLine();
					Box* find_box = board->box(x,y+1);
					if(find_box->isTypeAllFixed()){
						cout << "Conflict!! (error: 62)" << endl;
						exit(62);
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
				else if(d.w){
					trgt_box->fixWestLine();
					Box* find_box = board->box(x-1,y);
					if(find_box->isTypeAllFixed()){
						cout << "Conflict!! (error: 63)" << endl;
						exit(63);
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
}

void printFixFlag() {

	cout << "FIX FLAG" << endl;
	cout << "========" << endl;

	for (int y = 0; y < board->getSizeY(); y++) {
		for (int x = 0; x < board->getSizeX(); x++) {
			Box* trgt_box = board->box(x, y);
			if (trgt_box->isTypeAllFixed()) {
				cout << " ▮";
			} else if(trgt_box->isTypeHalfFixed()) {
				cout << " ▯";
			} else {
				cout << " -";
			}
		}
		cout << endl;
	}
	cout << endl;

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
