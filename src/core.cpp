#include "core.h"
#include "sound.h"

bool player_up = false;
bool player_down = false;
bool player_left = false;
bool player_right = false;
bool player_select = false;
bool player_start = false;
bool player_confirm = false;
bool player_cancel = false;
mutex transition;

extern soundengine se;

typedef std::map<string, std::map<string, float>>::iterator type_iter;
int min(int a, int b) {
	if (a < b) {
		return a;
	}
	return b;
}
float min(float a, float b) {
	if (a < b) {
		return a;
	}
	return b;
}
double min(double a, double b) {
	if (a < b) {
		return a;
	}
	return b;
}
int max(int a, int b) {
	if (a > b) {
		return a;
	}
	return b;
}
float max(float a, float b) {
	if (a > b) {
		return a;
	}
	return b;
}
double max(double a, double b) {
	if (a > b) {
		return a;
	}
	return b;
}

string get_direction_string(direction dir) {
	switch (dir) {
	case UP:
		return string("up");
	case DOWN:
		return string("down");
	case LEFT:
		return string("left");
	case RIGHT:
		return string("right");
	}
	return string("ERROR");
}

void engine::full_heal() {
	for (unsigned i = 0; i < 6; ++i) {
		if (mc.team[i].defined) {
			mc.team[i].curr_hp = get_stat(mc.team[i], HP);
			mc.team[i].status.clear();
			for (unsigned j = 0; j < 4; ++j) {
				if (moves[mc.team[i].moves[j]].defined) {
					mc.team[i].pp[j] = mc.team[i].max_pp[j];
				}
			}
		}
	}
}

string engine::get_item_effect(string in) {
	string out = "";
	if (!items[in].defined)
		return string("");
	for (unsigned i = 0; i < items[in].effect.size(); ++i) {
		if (out.size() > 0) {
			out = out + "|";
		}
		out = out + items[in].effect[i];
	}
	return out;
}

void engine::push_hp_bar_if_exists(float x, float y, int index) {
	if (mc.team[index].defined)
		g.push_hp_bar(x, y, get_hp_percent(mc.team[index]));
}

string engine::get_special_string(string in) {
	string parse;
	string out;
	bool number = false;
	if (in.length() == 0)
		return in;
	if (in.find(":") != -1) {
		string temp = in;
		for (unsigned i = 0; (i < temp.size()) && (temp[i] != ':'); ++i) {
			parse += temp[i];
		}
		temp.erase(0, temp.find(':') + 1);
		for (int i = 0; i <= 9; ++i) {
			if (temp.find(to_string(i)) == 0) {
				number = true;
				break;
			}
		}
		if (number && stoi(temp) == -1)
			return string(""); // TODO:  Better error detection.
		if (parse == "ACTIVE_MON_MOVE") {
			out = mc.team[mc.selected].moves[stoi(temp)];
			if (out == "") {
				out = "-";
			}
			return out;
		}
		else if (parse == "MON_MOVE") {
			int index1 = 0;
			int index2 = 0;
			index1 = stoi(temp);
			temp.erase(0, temp.find(":") + 1);
			index2 = stoi(temp);
			out = mc.team[index1].moves[index2];
			if (out == "") {
				out = "-";
			}
			return out;
		}
		else if (parse == "STORAGE_MON_MOVE") {
			int index1 = 0;
			int index2 = 0;
			index1 = stoi(temp);
			temp.erase(0, temp.find(":") + 1);
			index2 = stoi(temp);
			out = mc.storage[mc.box_number][index1].moves[index2];
			if (out == "") {
				out = "-";
			}
			return out;
		}
		else if (parse == "ACTIVE_MON_MOVE_TYPE") {
			if (!number)
				return in;
			return moves[mc.team[mc.selected].moves[stoi(temp)]].type;
		}
		else if (parse == "ACTIVE_MON_MOVE_PP") {
			if (!number)
				return in;
			int index = stoi(temp);
			out = to_string(mc.team[mc.selected].pp[index]) + string("/") + to_string(mc.team[mc.selected].max_pp[index]);
			if (in_status(mc.team[mc.selected], string("DISABLE")) && index == mc.team[mc.selected].disabled_move)
				out = "DISABLED";
			while (out.length() < 9)
				out = string(" ") + out;
			return out;
		}
		else if (parse == "MON_MOVE_PP_IF_EXISTS") {
			int index1 = 0;
			int index2 = 0;
			index1 = stoi(temp);
			temp.erase(0, temp.find(":") + 1);
			index2 = stoi(temp);
			if (!moves[mc.team[index1].moves[index2]].defined)
				return string(" ");
			return string("PP");
		}
		else if (parse == "STORAGE_MON_MOVE_PP_IF_EXISTS") {
			int index1 = 0;
			int index2 = 0;
			index1 = stoi(temp);
			temp.erase(0, temp.find(":") + 1);
			index2 = stoi(temp);
			if (!moves[mc.storage[mc.box_number][index1].moves[index2]].defined)
				return string(" ");
			return string("PP");
		}
		else if (parse == "MON_MOVE_PP") {
			int index1 = 0;
			int index2 = 0;
			index1 = stoi(temp);
			temp.erase(0, temp.find(":") + 1);
			index2 = stoi(temp);
			if (!moves[mc.team[index1].moves[index2]].defined)
				return string(" ");
			out = to_string(mc.team[index1].pp[index2]) + string("/") + to_string(mc.team[index1].max_pp[index2]);
			while (out.length() < 9)
				out = string(" ") + out;
			return out;
		}
		else if (parse == "STORAGE_MON_MOVE_PP") {
			int index1 = 0;
			int index2 = 0;
			index1 = stoi(temp);
			temp.erase(0, temp.find(":") + 1);
			index2 = stoi(temp);
			if (!moves[mc.storage[mc.box_number][index1].moves[index2]].defined)
				return string(" ");
			out = to_string(mc.storage[mc.box_number][index1].pp[index2]) + string("/") + to_string(mc.storage[mc.box_number][index1].max_pp[index2]);
			while (out.length() < 9)
				out = string(" ") + out;
			return out;
		}
		else if (parse == "TEAM_MON_NAME") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			return get_nickname(mc.team[index]);
		}
		else if (parse == "STORAGE_MON_NAME") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			return get_nickname(mc.storage[mc.box_number][index]);
		}
		else if (parse == "STORAGE_MON_LEVEL") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			if (mc.storage[mc.box_number][index].level == 100)
				return string("{LEVEL}**");
			return string("{LEVEL}") + to_string(mc.storage[mc.box_number][index].level);
		}
		else if (parse == "TEAM_MON_LEVEL") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			if (mc.team[index].level == 100)
				return string("{LEVEL}**");
			return string("{LEVEL}") + to_string(mc.team[index].level);
		}
		else if (parse == "TEAM_MON_CAN_LEARN") {
			string temp2 = temp;
			temp2.erase(temp2.find(":"), temp2.size());
			temp.erase(0, temp.find(":") + 1);
			int index = stoi(temp2);
			if (!mc.team[index].defined)
				return string("");
			if (temp.find("TM") != -1) {
				temp.erase(0, temp.find(":") + 1);
				if (all_mon[mc.team[index].number].TM[stoi(temp)])
					return string("ABLE");
				return string("NOT ABLE");
			}
			else if (temp.find("HM") != -1) {
				temp.erase(0, temp.find(":") + 1);
				if (all_mon[mc.team[index].number].HM[stoi(temp)])
					return string("ABLE");
				return string("NOT ABLE");
			}
			else
				return string("ERROR");
		}
		else if (parse == "TEAM_MON_CAN_EVOLVE") {
			string temp2 = temp;
			temp2.erase(temp2.find(":"), temp2.size());
			temp.erase(0, temp.find(":") + 1);
			int index = stoi(temp2);
			if (!mc.team[index].defined)
				return string("");
			if (can_evolve(mc.team[index].number, temp))
				return string("ABLE");
			return string("NOT ABLE");
		}
		else if (parse == "TEAM_MON_CAN_EVOLVE_FOLLOW_UP") {
			string temp2 = temp;
			temp2.erase(temp2.find(":"), temp2.size());
			temp.erase(0, temp.find(":") + 1);
			int index = stoi(temp2);
			if (can_evolve(mc.team[index].number, temp))
				return string("");
			return string("ALERT:It didn't seem to have any effect.");
		}
		else if (parse == "TEAM_MON_CAN_LEARN_FOLLOW_UP") {
			string temp2 = temp;
			temp2.erase(temp2.find(":"), temp2.size());
			temp.erase(0, temp.find(":") + 1);
			int index = stoi(temp2);
			if (!mc.team[index].defined)
				return string("");
			if (temp.find("TM") != -1) {
				temp.erase(0, temp.find(":") + 1);
				if (all_mon[mc.team[index].number].TM[stoi(temp)])
					return string("");
				return string("ALERT:") + get_nickname(mc.team[index]) + string(" is not compatible with ") + TM[stoi(temp)] + string(".  It can't learn ") + TM[stoi(temp)] + string(".");
			}
			else if (temp.find("HM") != -1) {
				temp.erase(0, temp.find(":") + 1);
				if (all_mon[mc.team[index].number].HM[stoi(temp)])
					return string("");
				return string("ALERT:") + get_nickname(mc.team[index]) + string(" is not compatible with ") + HM[stoi(temp)] + string(".  It can't learn ") + HM[stoi(temp)] + string(".");
			}
			else
				return string("");
		}
		else if (parse == "TEAM_MON_MAX_HP") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			return to_string(get_stat(mc.team[index], HP));
		}
		else if (parse == "TEAM_MON_NUMBER") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			return string("{NO}.") + mc.team[index].number;
		}
		else if (parse == "STORAGE_MON_NUMBER") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			return string("{NO}.") + mc.storage[mc.box_number][index].number;
		}
		else if (parse == "TEAM_MON_IMAGE") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			return mc.team[index].number + string(".png");
		}
		else if (parse == "STORAGE_MON_IMAGE") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			return mc.storage[mc.box_number][index].number + string(".png");
		}
		else if (parse == "TEAM_MON_CURRENT_HP") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			return to_string(mc.team[index].curr_hp);
		}
		else if (parse == "TEAM_MON_FORMATTED_HP") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			string o = to_string(mc.team[index].curr_hp) + string("/") + to_string(get_stat(mc.team[index], HP));
			while (o.length() < 7)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "STORAGE_MON_FORMATTED_HP") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			string o = to_string(mc.storage[mc.box_number][index].curr_hp) + string("/") + to_string(get_stat(mc.storage[mc.box_number][index], HP));
			while (o.length() < 7)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "TEAM_MON_ATTACK") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			string o = to_string(get_stat(mc.team[index], ATTACK, true, true));
			while (o.length() < 8)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "STORAGE_MON_ATTACK") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			string o = to_string(get_stat(mc.storage[mc.box_number][index], ATTACK, true, true));
			while (o.length() < 8)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "MON_TEMPLATE_NAME") {
			if (!all_mon[temp].defined)
				return string("");
			return all_mon[temp].name;
		}
		else if (parse == "MON_ENTRY") {
			if (!all_mon[temp].defined)
				return string("");
			return all_mon[temp].dex;
		}
		else if (parse == "MON_WEIGHT") {
			if (!all_mon[temp].defined)
				return string("");
			string o = to_string(all_mon[temp].weight);
			string out = "";
			for (unsigned i = 0; i < o.size(); ++i) {
				out = out + o[i];
				if (o[i] == '.') {
					out = out + o[i + 1];
					break;
				}
			}
			return out + string("lb");
		}
		else if (parse == "MON_HEIGHT") {
			if (!all_mon[temp].defined)
				return string("");
			int index = all_mon[temp].height;
			string o = "";
			if (index >= 12)
				o = o + to_string(index / 12) + string("{FEET}");
			o = o + to_string(index % 12) + string("{INCHES}");
			return o;
		}
		else if (parse == "TEAM_MON_DEFENSE") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			string o = to_string(get_stat(mc.team[index], DEFENSE, true, true));
			while (o.length() < 8)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "STORAGE_MON_DEFENSE") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			string o = to_string(get_stat(mc.storage[mc.box_number][index], DEFENSE, true, true));
			while (o.length() < 8)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "TEAM_MON_SPEED") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			string o = to_string(get_stat(mc.team[index], SPEED, true, true));
			while (o.length() < 8)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "STORAGE_MON_SPEED") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			string o = to_string(get_stat(mc.storage[mc.box_number][index], SPEED, true, true));
			while (o.length() < 8)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "TEAM_MON_SPECIAL") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			string o = to_string(get_stat(mc.team[index], SPECIAL, true, true));
			while (o.length() < 8)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "STORAGE_MON_SPECIAL") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			string o = to_string(get_stat(mc.storage[mc.box_number][index], SPECIAL, true, true));
			while (o.length() < 8)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "TEAM_MON_TYPE1") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			return get_type_1(mc.team[index]);
		}
		else if (parse == "STORAGE_MON_TYPE1") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			return get_type_1(mc.storage[mc.box_number][index]);
		}
		else if (parse == "TEAM_MON_TYPE2") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			return get_type_2(mc.team[index]);
		}
		else if (parse == "STORAGE_MON_TYPE2") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			return get_type_2(mc.storage[mc.box_number][index]);
		}
		else if (parse == "TYPE2_IF_EXISTS") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			if (get_type_2(mc.team[index]) != "")
				return string("TYPE2/");
			return string("");
		}
		else if (parse == "STORAGE_TYPE2_IF_EXISTS") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			if (get_type_2(mc.storage[mc.box_number][index]) != "")
				return string("TYPE2/");
			return string("");
		}
		else if (parse == "STATUS_BOOL") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			if (mc.team[index].curr_hp <= 0)
				return string("STATUS/KO");
			if (mc.team[index].status.size() != 0)
				return string("STATUS/BAD");
			return string("STATUS/OK");
		}
		else if (parse == "STORAGE_STATUS_BOOL") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			if (mc.storage[mc.box_number][index].curr_hp <= 0)
				return string("STATUS/KO");
			if (mc.storage[mc.box_number][index].status.size() != 0)
				return string("STATUS/BAD");
			return string("STATUS/OK");
		}
		else if (parse == "TEAM_MON_STATUS") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			if (mc.team[index].curr_hp <= 0)
				return string("KO");
			string o = "{SINGLE}";
			for (unsigned i = 0; i < mc.team[index].status.size(); ++i) {
				if (status[mc.team[index].status[i]].defined)
					if (status[mc.team[index].status[i]].nonvolatile)
						if (o.find(mc.team[index].status[i]) == -1)
							o = o + mc.team[index].status[i] + string("{NEWLINE}");
			}
			if (o == "{SINGLE}")
				o = "{SINGLE}OK";
			return o;
		}
		else if (parse == "STORAGE_MON_STATUS") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			if (mc.storage[mc.box_number][index].curr_hp <= 0)
				return string("KO");
			string o = "{SINGLE}";
			for (unsigned i = 0; i < mc.storage[mc.box_number][index].status.size(); ++i) {
				if (status[mc.storage[mc.box_number][index].status[i]].defined)
					if (status[mc.storage[mc.box_number][index].status[i]].nonvolatile)
						if (o.find(mc.storage[mc.box_number][index].status[i]) == -1)
							o = o + mc.storage[mc.box_number][index].status[i] + string("\n");
			}
			if (o == "{SINGLE}")
				o = "{SINGLE}OK";
			return o;
		}
		else if (parse == "TEAM_MON_NAME_RIGHT_JUSTIFIED") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			string o = get_nickname(mc.team[index]);
			while (o.length() < 14)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "STORAGE_MON_NAME_RIGHT_JUSTIFIED") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			string o = get_nickname(mc.storage[mc.box_number][index]);
			while (o.length() < 14)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "EXP_POINTS") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			string o = to_string(mc.team[index].exp);
			while (o.length() < 10)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "STORAGE_EXP_POINTS") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			string o = to_string(mc.storage[mc.box_number][index].exp);
			while (o.length() < 10)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "LEVEL_UP") {
			int index = stoi(temp);
			if (!mc.team[index].defined)
				return string("");
			int tot = -1;
			for (int i = 0; i <= 100; ++i) {
				if (e.level_to_exp[i] > mc.team[index].exp) {
					tot = e.level_to_exp[i] - mc.team[index].exp;
					break;
				}
			}
			string o = string("");
			if (tot != -1) {
				o = to_string(tot);
			}
			while (o.length() < 10)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "STORAGE_LEVEL_UP") {
			int index = stoi(temp);
			if (!mc.storage[mc.box_number][index].defined)
				return string("");
			int tot = -1;
			for (int i = 0; i <= 100; ++i) {
				if (e.level_to_exp[i] > mc.storage[mc.box_number][index].exp) {
					tot = e.level_to_exp[i] - mc.storage[mc.box_number][index].exp;
					break;
				}
			}
			string o = string("");
			if (tot != -1) {
				o = to_string(tot);
			}
			while (o.length() < 10)
				o = string(" ") + o;
			return o;
		}
		else if (parse == "RIGHT_JUSTIFY") {
			unsigned len = stoi(temp);
			temp.erase(0, temp.find(':') + 1);
			temp = get_special_string(temp);
			unsigned real_len = 0;
			for (unsigned i = 0; i < temp.length(); ++i) {
				real_len++;
				if (temp[i] == '{') {
					while (temp[i] != '}') {
						++i;
					}
				}
			}
			while (real_len < len) {
				temp = string(" ") + temp;
				real_len++;
			}
			return temp;
		}
		else if (parse == "CENTER_JUSTIFY") {
			unsigned len = stoi(temp);
			temp.erase(0, temp.find(':') + 1);
			temp = get_special_string(temp);
			bool front = true;
			while (temp.length() < len) {
				if (front) {
					temp = string(" ") + temp;
				}
				else {
					temp = temp + string(" ");
				}
				front = !front;
			}
			return temp;
		}
		else if (parse == "MOVE_POWER") {
			if (!moves[temp].defined)
				return string("-");
			return moves[temp].pow;
		}
		else if (parse == "MOVE_ACC") {
			if (!moves[temp].defined)
				return string("-");
			return to_string(moves[temp].acc);
		}
		else if (parse == "MOVE_TYPE") {
			if (!moves[temp].defined)
				return string("-");
			return moves[temp].type;
		}
		else if (parse == "MOVE_DEFENSE") {
			if (!moves[temp].defined)
				return string("-");
			switch (moves[temp].defense) {
			case ATTACK:
				return string("ATTACK");
			case DEFENSE:
				return string("DEFENSE");
			case SPECIAL:
				return string("SPECIAL");
			case SPEED:
				return string("SPEED");
			default:
				return string("-");
			}
		}
		else if (parse == "MOVE_CATEGORY") {
			if (!moves[temp].defined)
				return string("-");
			switch (moves[temp].defense) {
			case DEFENSE:
				return string("PHYSICAL");
			case SPECIAL:
				return string("SPECIAL");
			default:
				return string("-");
			}
		}
		else if (parse == "MOVE_NOTES") {
			if (!moves[temp].defined)
				return string("-");
			return moves[temp].notes;
		}
		else if (parse == "ITEM") {
			string type = temp;
			type.erase(type.find(":"), type.length());
			temp.erase(0, temp.find(":") + 1);
			int index = stoi(temp);
			return get_item_name(type, index);
		}
		else if (parse == "ITEM_COUNT") {
			string type = temp;
			type.erase(type.find(":"), type.length());
			temp.erase(0, temp.find(":") + 1);
			int index = stoi(temp);
			return get_item_count(type, index);
		}
		else if (parse == "ITEM_STORAGE") {
			string type = temp;
			type.erase(type.find(":"), type.length());
			temp.erase(0, temp.find(":") + 1);
			int index = stoi(temp);
			return get_item_storage_name(type, index);
		}
		else if (parse == "ITEM_STORAGE_COUNT") {
			string type = temp;
			type.erase(type.find(":"), type.length());
			temp.erase(0, temp.find(":") + 1);
			int index = stoi(temp);
			return get_item_storage_count(type, index);
		}
	}
	if (in == "{RIVAL_NAME}") {
		return mc.rivalname;
	}
	else if (in == "{PLAYER_NAME}") {
		return mc.name;
	}
	else if (in.find("{BOX_NUMBER}") != -1) {
		string out = to_string(mc.box_number + 1);
		if (mc.box_number < 9)
			out = string(" ") + out;
		in.insert(in.find("{BOX_NUMBER}"), out);
		in.erase(in.find("{BOX_NUMBER}"), string("{BOX_NUMBER}").length());
		return in;
	}
	else if (in == "{MONEY}") {
		return string("$") + to_string(mc.money);
	}
	else if (in == "{COINS}") {
		return to_string(mc.coins);
	}
	else if (in == "{SEEN}") {
		std::map<string, bool>::iterator it = mc.seen.begin();
		unsigned count = 0;
		while (it != mc.seen.end()) {
			if (it->second)
				count++;
			it++;
		}
		return to_string(count);
	}
	else if (in == "{CAUGHT}") {
		std::map<string, bool>::iterator it = mc.caught.begin();
		unsigned count = 0;
		while (it != mc.caught.end()) {
			if (it->second)
				count++;
			it++;
		}
		return to_string(count);
	}
	return in;
}

bool engine::can_use_item(string filter, string name) {
	if (!items[name].defined)
		return false;
	if (items[name].effect.size() == 0)
		return false;
	for (unsigned i = 0; i < items[name].use.size(); ++i) {
		if (items[name].use[i] == filter)
			return true;
	}
	return false;
}

bool engine::use_item(string filter, std::vector<int> &choices, string &ret) {
	ret = string("");
	string name = get_item_name(filter, choices[1]);
	string effect = get_item_effect(name);
	string base = effect;
	while (base.size() > 0) {
		base = effect;
		if (base.find("|") != -1) {
			effect.erase(0, effect.find("|") + 1);
			base.erase(base.find("|"), base.size());
		}
		else {
			effect = "";
		}
		if (is_menu(base)) {
			string menu = base;
			menu.erase(menu.find(":"), menu.size());
			base.erase(0, base.find(":") + 1);
			if (!do_effect(mc.team[choices[2]], base, choices[choices.size() - 1]))
				return false;
		}
		else if (base.find("CAPTURE") == 0) {
			ret = base;
		}
		else if (base.find("FISH") == 0) {
			location temp = mc.loc;
			if (mc.dir == UP) {
				temp.y -= 1.0;
			}
			if (mc.dir == DOWN) {
				temp.y += 1.0;
			}
			if (mc.dir == LEFT) {
				temp.x -= 1.0;
			}
			if (mc.dir == RIGHT) {
				temp.x += 1.0;
			}
			if (!water[get_tile(temp.y, temp.x)]) {
				return false;
			}
			string s = base;
			s.erase(0, s.find(":") + 1);
			if (s.find("|") != -1) {
				s.erase(s.find("|"), s.length());
			}
			int tier = stoi(s);
			bool first = int(random(0.0, 2.0)) == 1;
			vector<int> combination;
			vector<int> base;
			for (unsigned i = 0; i < levels[mc.loc.level].encounters.size(); ++i) {
				combination.push_back(levels[mc.loc.level].encounters[i]);
			}
			if (tier == 0) {
				for (unsigned i = 0; i < levels[mc.loc.level].water_encounters_old.size(); ++i) {
					combination.push_back(levels[mc.loc.level].water_encounters_old[i]);
					base.push_back(levels[mc.loc.level].water_encounters_old[i]);
				}
			}
			else if (tier == 1) {
				for (unsigned i = 0; i < levels[mc.loc.level].water_encounters_good.size(); ++i) {
					combination.push_back(levels[mc.loc.level].water_encounters_good[i]);
					base.push_back(levels[mc.loc.level].water_encounters_good[i]);
				}
			}
			else if (tier == 2) {
				for (unsigned i = 0; i < levels[mc.loc.level].water_encounters_super.size(); ++i) {
					combination.push_back(levels[mc.loc.level].water_encounters_super[i]);
					base.push_back(levels[mc.loc.level].water_encounters_super[i]);
				}
			}
			int part1 = combination[int(random(0.0, combination.size()))];
			int part2 = base[int(random(0.0, base.size()))];
			if (first) {
				encounter = to_string(part1) + string("-") + to_string(part2);
			}
			else {
				encounter = to_string(part2) + string("-") + to_string(part1);
			}
			encounter_level = int(random(levels[mc.loc.level].level_range.first, levels[mc.loc.level].level_range.second + 1));
			if (tier == 0) {
				encounter_level = 5;
			}
			else if (tier == 1) {
				encounter_level = max(encounter_level - 5, 5);
			}
			//TODO: Random chance for fishing to work
		}
		else if (base.find("MAP") == 0) {
			// TODO: MAP
		}
		else if (base.find("BIKE") == 0) {
			// TODO: BIKE
		}
		else if (base.find("ITEMFINDER") == 0) {
			// TODO: ITEMFINDER
		}
		else if (base.find("EVOLVE") == 0) {
			base.erase(0, base.find(":") + 1);
			choices = do_menu("EVOLVE_MON_SELECT", base);
			choices = remove_cancels(choices);
			if (choices.size() <= 0)
				return false;
			if (!can_evolve(mc.team[choices[0]].number, base))
				return false;
			for (unsigned i = 0; i < all_mon[mc.team[choices[0]].number].evolution.size(); ++i) {
				if (all_mon[mc.team[choices[0]].number].evolution[i].second == base) {
					// TODO:  EVOLUTION SCREEN
					do_alert(string("What? ") + get_nickname(mc.team[choices[0]]) + string(" is evolving!"));
					string old_nickname = get_nickname(mc.team[choices[0]]);
					mc.team[choices[0]].number = all_mon[mc.team[choices[0]].number].evolution[i].first;
					do_alert(old_nickname + string(" evolved into ") + all_mon[mc.team[choices[0]].number].name + string("!"));
					mc.seen[mc.team[choices[0]].number] = true;
					mc.caught[mc.team[choices[0]].number] = true;
					break;
				}
			}
		}
		else if (base.find("TM") == 0) {
			string move = base;
			string record = base;
			move.erase(0, move.find(":") + 1);
			move = TM[stoi(move)];
			do_menu(string("ALERT"), string("Booted up a TM!"));
			do_menu(string("ALERT"), string("It contained ") + move + string("!"));
			choices = do_menu(string("ALERT_YES_NO"), string("Teach ") + move + string(" to a POK{e-accent}MON?"));
			choices = remove_cancels(choices);
			if (choices[0] == 1)
				return false;
			choices = do_menu(string("LEARN_MON_SELECT"), base);
			if (choices.size() > 1)
				return false;
			if (choices[0] == -1)
				return false;
			bool out = learn_move(mc.team[choices[0]], move);
			if (out) {
				while (record.find(":") != -1) {
					record.erase(record.find(":"), 1);
				}
				mc.used_tms[record] = true;
			}
		}
		else if (base.find("HM") == 0) {
			string move = base;
			move.erase(0, move.find(":") + 1);
			move = HM[stoi(move)];
			do_menu(string("ALERT"), string("Booted up an HM!"));
			do_menu(string("ALERT"), string("It contained ") + move + string("!"));
			choices = do_menu(string("ALERT_YES_NO"), string("Teach ") + move + string(" to a POK{e-accent}MON?"));
			if (choices[choices.size() - 1] == 1)
				return false;
			choices = do_menu(string("LEARN_MON_SELECT"), base);
			if (choices.size() > 1)
				return false;
			if (choices[0] == -1)
				return false;
			bool out = learn_move(mc.team[choices[0]], move);
		}
		else if (base.find("TARGET") == 0) {
			do_effect(mc.enemy_team[mc.enemy_selected], base);
		}
		else if (base.find("TEAM") == 0) {
			for (int i = 0; i < 6; ++i) {
				if (mc.team[i].defined)
					do_effect(mc.team[i], base);
			}
		}
		else if (base.find("SELF") == 0) {
			do_effect(mc.team[mc.selected], base);
		}
		else if (base.find("TELEPORT") == 0) {
			ret = "TELEPORT";
		}
		else if (base.find("FLEE") == 0) {
			ret = "FLEE";
		}
		else if (base.find("REPEL") == 0) {
			base.erase(0, base.find(":") + 1);
			mc.repel += stoi(base);
		}
	}
	if (get_item_effect(name).find("INFINITE") == -1) {
		for (unsigned i = 0; i < mc.inventory.size(); ++i) {
			if (mc.inventory[i].first == name) {
				mc.inventory[i].second--;
				if (mc.inventory[i].second <= 0) {
					mc.inventory.erase(mc.inventory.begin() + i);
				}
				break;
			}
		}
	}
	return true;
}

bool engine::do_effect(mon& m, string effect, int extra) {
	if (effect.find("|") != -1)
		effect.erase(effect.find("|"), effect.size());
	if (effect.find("SELF") == 0) {
		effect.erase(0, effect.find(":") + 1);
	}
	if (effect.find("TARGET") == 0) {
		effect.erase(0, effect.find(":") + 1);
	}
	if (effect.find("TEAM") == 0) {
		effect.erase(0, effect.find(":") + 1);
	}
	if (effect.find("CLEAR_STATUS") == 0) {
		if (effect.find(":") != -1) {
			effect.erase(0, effect.find(":") + 1);
			remove_status(m, effect, true);
		}
		else {
			while (m.status.size() > 0) {
				remove_status(m, m.status[0], true);
			}
		}
	}
	else if (effect.find("FILL_HP") == 0) {
		effect.erase(0, effect.find(":") + 1);
		if (!is_KO(m)) {
			heal_damage(m, stoi(effect));
		}
		else {
			do_alert(string("It's knocked out cold!"));
			return false;
		}
	}
	else if (effect.find("FILL_PP") == 0) {
		effect.erase(0, effect.find(":") + 1);
		m.pp[extra] = min(m.pp[extra] + stoi(effect), m.max_pp[extra]);
	}
	else if (effect.find("FILL_ALL_PP") == 0) {
		effect.erase(0, effect.find(":") + 1);
		for (unsigned i = 0; i < 4; i++) {
			if (moves[m.moves[i]].defined) {
				m.pp[i] = min(m.pp[i] + stoi(effect), m.max_pp[i]);
			}
		}
	}
	else if (effect.find("REVIVE") == 0) {
		if (m.curr_hp > 0)
			return false;
		m.curr_hp = get_stat(m, HP) / 2;
		m.status.clear();
	}
	else if (effect.find("EV_UP") == 0) {
		STAT s;
		effect.erase(0, effect.find(":") + 1);
		if (effect.find("HP") == 0) {
			s = HP;
		}
		else if (effect.find("ATTACK") == 0) {
			s = ATTACK;
		}
		else if (effect.find("DEFENSE") == 0) {
			s = DEFENSE;
		}
		else if (effect.find("SPEED") == 0) {
			s = SPEED;
		}
		else if (effect.find("SPECIAL") == 0) {
			s = SPECIAL;
		}
		else {
			return false;
		}
		effect.erase(0, effect.find(":") + 1);
		// TODO: Add in-game EV boost limit
		m.EV[s] += stoi(effect);
	}
	else if (effect.find("PP_UP") == 0) {
		int i = int(double(moves[m.moves[extra]].pp) * 0.2) + m.max_pp[extra];
		int j = int(double(moves[m.moves[extra]].pp) * 1.6);
		i = min(i, j);
		m.max_pp[extra] = i;
		m.pp[extra] = i;
		// TODO:  Don't use up items from maxed out PP.
	}
	else if (effect.find("LEVEL_UP") == 0) {
		// TODO: Fail when level 100
		m.exp = level_to_exp[min(100, m.level + 1)];
		level_up(m, true);
	}
	else if (effect.find("APPLY_STATUS") == 0) {
		effect.erase(0, effect.find(":") + 1);
		apply_status(m, effect);
	}
	else if (effect.find("TM") == 0) {
		effect.erase(0, effect.find(":") + 1);
		if (!learn_move(m, TM[stoi(effect)]))
			return false;
	}
	else if (effect.find("HM") == 0) {
		effect.erase(0, effect.find(":") + 1);
		if (!learn_move(m, HM[stoi(effect)]))
			return false;
	}
	return true;
}

string engine::get_item_name(string type, int index) {
	unsigned i = 0, count = 0;
	while (count < e.mc.inventory.size()) {
		bool found = false;
		if (type == "ALL")
			found = true;
		for (unsigned j = 0; j < e.items[e.mc.inventory[count].first].use.size(); ++j) {
			if (type == e.items[e.mc.inventory[count].first].use[j])
				found = true;
		}
		if (!found) {
			count++;
			continue;
		}
		if (i == index) {
			return e.mc.inventory[count].first;
		}
		i++;
		count++;
	}
	return string("NOT FOUND");
}

string engine::get_item_storage_name(string type, int index) {
	unsigned i = 0, count = 0;
	while (count < e.mc.inventory_storage.size()) {
		bool found = false;
		if (type == "ALL")
			found = true;
		for (unsigned j = 0; j < e.items[e.mc.inventory_storage[count].first].use.size(); ++j) {
			if (type == e.items[e.mc.inventory_storage[count].first].use[j])
				found = true;
		}
		if (!found) {
			count++;
			continue;
		}
		if (i == index) {
			return e.mc.inventory_storage[count].first;
		}
		i++;
		count++;
	}
	return string("NOT FOUND");
}

int engine::get_item_cost(string in) {
	return e.items[in].price;
}

int engine::get_inventory_count(string in) {
	for (unsigned i = 0; i < mc.inventory.size(); ++i) {
		if (in == mc.inventory[i].first)
			return mc.inventory[i].second;
	}
	return 0;
}

int engine::get_inventory_storage_count(string in) {
	for (unsigned i = 0; i < mc.inventory_storage.size(); ++i) {
		if (in == mc.inventory_storage[i].first)
			return mc.inventory_storage[i].second;
	}
	return 0;
}

string engine::get_item_count(string type, int index) {
	unsigned i = 0, count = 0;
	while (count < e.mc.inventory.size()) {
		bool found = false;
		if (type == "ALL")
			found = true;
		for (unsigned j = 0; j < e.items[e.mc.inventory[count].first].use.size(); ++j) {
			if (type == e.items[e.mc.inventory[count].first].use[j])
				found = true;
		}
		if (!found) {
			count++;
			continue;
		}
		if (i == index) {
			return string("{TIMES}") + to_string(e.mc.inventory[count].second);
		}
		i++;
		count++;
	}
	return string("NOT FOUND");
}

string engine::get_item_storage_count(string type, int index) {
	unsigned i = 0, count = 0;
	while (count < e.mc.inventory_storage.size()) {
		bool found = false;
		if (type == "ALL")
			found = true;
		for (unsigned j = 0; j < e.items[e.mc.inventory_storage[count].first].use.size(); ++j) {
			if (type == e.items[e.mc.inventory_storage[count].first].use[j])
				found = true;
		}
		if (!found) {
			count++;
			continue;
		}
		if (i == index) {
			return string("{TIMES}") + to_string(e.mc.inventory_storage[count].second);
		}
		i++;
		count++;
	}
	return string("NOT FOUND");
}

bool engine::withdraw_item(string s, int count) {
	bool ret = true;
	while (count > 0) {
		ret = ret & withdraw_item(s);
		count--;
	}
	return ret;
}

bool engine::withdraw_item(string s) {
	for (unsigned i = 0; i < mc.inventory_storage.size(); ++i) {
		if (mc.inventory_storage[i].first == s) {
			mc.inventory_storage[i].second--;
			if (mc.inventory_storage[i].second <= 0) {
				mc.inventory_storage.erase(mc.inventory_storage.begin() + i);
			}
			return true;
		}
	}
	return false;
}

bool engine::remove_item(string s, int count) {
	bool ret = true;
	while (count > 0) {
		ret = ret & remove_item(s);
		count--;
	}
	return ret;
}

bool engine::remove_item(string s) {
	for (unsigned i = 0; i < mc.inventory.size(); ++i) {
		if (mc.inventory[i].first == s) {
			mc.inventory[i].second--;
			if (mc.inventory[i].second <= 0) {
				mc.inventory.erase(mc.inventory.begin() + i);
			}
			return true;
		}
	}
	return false;
}

bool engine::store_item(string s, int count) {
	bool found = false;
	if (!items[s].defined) {
		do_menu(string("ALERT"), string("Game attempted to store bad item: ") + s);
		return false;
	}
	for (unsigned i = 0; i < mc.inventory_storage.size(); ++i) {
		if (mc.inventory_storage[i].first == s) {
			found = true;
			mc.inventory_storage[i].second += count;
			break;
		}
	}
	if (!found) {
		pair<string, int> item;
		item.first = s;
		item.second = count;
		mc.inventory_storage.push_back(item);
	}
	return true;
}

bool engine::gain_item(string s, int count, bool silent) {
	bool found = false;
	if (!items[s].defined) {
		do_menu(string("ALERT"), string("Game attempted to give bad item: ") + s + string(" to player."));
		return false;
	}
	for (unsigned i = 0; i < mc.inventory.size(); ++i) {
		if (mc.inventory[i].first == s) {
			found = true;
			mc.inventory[i].second += count;
			break;
		}
	}
	if (!found) {
		pair<string, int> item;
		item.first = s;
		item.second = count;
		mc.inventory.push_back(item);
	}
	if (!silent)
		do_menu(string("ALERT"), string("{PLAYER_NAME} got ") + s + string("!"));
	return true;
}

bool engine::run_away(mon& escapee, mon& m, int attempts) {
	double chance = (double(get_stat(escapee, SPEED))*32.0 / double(get_stat(m, SPEED))) + (30.0 * double(attempts));
	return chance > random(0.0, 255.0);
}

int engine::has_move_in_party(string s) {
	bool found_move = false;
	for (unsigned i = 0; i < 6; ++i) {
		if (mc.team[i].defined) {
			if (has_move(mc.team[i], s)) {
				found_move = true;
			}
		}
	}
	if (found_move)
		return 1;
	return 0;
}

int engine::in_inventory(string s) {
	for (unsigned i = 0; i < mc.inventory.size(); ++i) {
		if (mc.inventory[i].first == s) {
			return min(mc.inventory[i].second, 1);
		}
	}
	return 0;
}

bool engine::in_status(mon& m, string s) {
	for (unsigned i = 0; i < m.status.size(); ++i) {
		if (m.status[i] == s) {
			return true;
		}
	}
	return false;
}

bool engine::in_special(string move, string s) {
	for (unsigned i = 0; i < moves[move].special.size(); ++i) {
		if (moves[move].special[i] == s)
			return true;
	}
	return false;
}

bool engine::in_move_target(string move, string effect) {
	for (unsigned i = 0; i < moves[move].target.size(); ++i) {
		if (moves[move].target[i] == effect)
			return true;
	}
	return false;
}

bool engine::in_move_self(string move, string effect) {
	for (unsigned i = 0; i < moves[move].self.size(); ++i) {
		if (moves[move].self[i] == effect)
			return true;
	}
	return false;
}

bool engine::in_move_special(string move, string effect) {
	for (unsigned i = 0; i < moves[move].special.size(); ++i) {
		if (moves[move].special[i] == effect)
			return true;
	}
	return false;
}

void engine::swap_mon(mon& a, mon& b) {
	mon temp;
	temp = a;
	a = b;
	b = temp;
}

int engine::attempt_capture(float capture_power, mon& m) {
	int ret = 0;
	double chance, hard_probability;
	chance = 3.0*double(get_stat(m, HP));
	chance -= 2.0*double(m.curr_hp);
	chance *= double(all_mon[m.number].catchrate);
	chance *= capture_power;
	if (in_status(m, string("POISON")))
		chance *= 1.5;
	if (in_status(m, string("TOXIC")))
		chance *= 1.5;
	if (in_status(m, string("BURN")))
		chance *= 1.5;
	if (in_status(m, string("PARALYZE")))
		chance *= 1.5;
	if (in_status(m, string("SLEEP")))
		chance *= 2.0;
	if (in_status(m, string("FREEZE")))
		chance *= 2.0;
	chance /= 3.0*double(get_stat(m, HP));
	chance = 1048560.0 / sqrt(sqrt(16711680.0 / chance));
	hard_probability = pow(chance / 65536.0, 4.0); // Debugging purposes only.
	for (ret = 0; ret < 4; ++ret) {
		if (random(0.0, 65536.0) > chance) {
			break;
		}
	}
	return ret;
}

string engine::get_nickname(mon& m) {
	if (m.nickname == "")
		return all_mon[m.number].name;
	return m.nickname;
}

void engine::make_mon(string ID, int e_level, mon& out) {
	out.level = 0;
	out.status.clear();
	out.last_move = "";
	out.exp = level_to_exp[e_level];
	if (out.exp == -1)
		int lkjawgjkalsgkj = 0;
	for (int x = 0; x < SIZE; x++)
		out.EV[x] = 0;
	for (int x = 0; x < SIZE; x++)
		out.IV[x] = int(random(0.0, 16.0));
	out.number = ID;
	out.wild = true;
	out.enemy = true;
	out.defined = true;
	out.turn_count = 0;
	out.nickname = "";
	for (int x = 0; x < 4; ++x)
		out.moves[x] = "";
	level_up(out);
	out.curr_hp = get_stat(out, HP);
	for (int x = 0; x < 6; ++x) {
		out.fought[x] = false;
	}
	if (out.level == 0) {
		make_mon(ID, e_level, out);
	}
	out.status.clear();
}

void engine::gain_exp(mon& winner, mon& loser, int num_fighters, double scale) {
	if (winner.original) {
		gain_exp(*winner.original, loser, num_fighters);
		return;
	}
	if (loser.original) {
		gain_exp(winner, *loser.original, num_fighters);
		return;
	}
	double exp = 1.0;
	if (!loser.wild)
		exp *= 1.5;
	exp *= all_mon[loser.number].exp_yield;
	exp *= loser.level;
	exp /= 5.0;
	exp /= double(num_fighters);
	exp *= pow((2.0*double(loser.level) + 10.0), 2.5);
	exp /= pow((double(winner.level + loser.level) + 10.0), 2.5);
	exp += 1.0;
	exp *= scale;
	double diff = double(max(min(winner.level - loser.level, 5), -10)) / 10.0;
	exp *= (1.0 - diff);
	exp = max(int(exp), 1);
	winner.exp += int(exp);
	for (unsigned i = 0; i < SIZE; ++i) {
		winner.EV[i] += all_mon[loser.number].stats[i];
	}
	resize_exp_bar(winner);
	do_alert(get_nickname(winner) + string(" gained ") + to_string(int(exp)) + string(" EXP. Points!"));
	level_up(winner, true);
	resize_exp_bar(winner);
}

int engine::get_move_count(mon& m) {
	int counter = 0;
	while ((counter < 4) && (m.moves[counter] != ""))
		counter++;
	return counter;
}

bool engine::level_up(mon& out, bool confirm_learn) {
	bool evolved = false;
	if (level_to_exp[out.level + 1] > out.exp)
		return false;
	int counter = get_move_count(out);
	while (out.level < 100 && level_to_exp[out.level + 1] <= out.exp) {
		out.level++;
		if (confirm_learn) {
			se.play_sound(string("sound_effects/general/sfx_level_up.mp3"));
			do_alert(get_nickname(out) + string(" grew to level ") + to_string(out.level) + "!");
			for (unsigned x = 0; x < all_mon[out.number].evolution.size(); ++x) {
				if (all_mon[out.number].evolution[x].second.find("LEVEL") == 0) {
					string temp;
					int min_level;
					temp = all_mon[out.number].evolution[x].second;
					temp.erase(0, temp.find(" ") + 1);
					min_level = stoi(temp);
					if (min_level <= out.level) {
						do_alert(string("What? ") + get_nickname(out) + string(" is evolving!"));
						string old_nickname = get_nickname(out);
						// TODO: Evolution screen and cancel option;
						out.number = all_mon[out.number].evolution[x].first;
						evolved = true;
						do_alert(old_nickname + string(" evolved into ") + all_mon[out.number].name + string("!"));
						mc.seen[out.number] = true;
						mc.caught[out.number] = true;
						rebuild_battle_hud(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected]);
					}
				}
			}
		}
		for (unsigned x = 0; x < all_mon[out.number].learned.size(); ++x) {
			if (all_mon[out.number].learned[x].first == out.level || (all_mon[out.number].learned[x].first == 0 && evolved) || (all_mon[out.number].learned[x].first == 0 && !confirm_learn && out.level == 1)) {
				if (!confirm_learn) { // TODO:  OR Surface confirmation window
					create_move(out, all_mon[out.number].learned[x].second, counter % 4);
					counter++;
				}
				else {
					learn_move(out, all_mon[out.number].learned[x].second);
				}
			}
		}
	}
	return true;
}

bool engine::learn_move(mon& m, string move) {
	vector<int> choices;
	if (m.wild) {
		return false;
	}
	for (int i = 0; i < 4; ++i) {
		if (m.moves[i] == move)
			return false;
	}
	int counter = get_move_count(m);
	if (counter < 4) {
		create_move(m, move, counter);
		do_menu("ALERT", get_nickname(m) + string(" learned ") + move + string("!"));
	}
	else {
		do_menu("ALERT", get_nickname(m) + string(" is trying to learn ") + move + string("!"));
		do_menu("ALERT", string("But ") + get_nickname(m) + string(" can't learn more than 4 moves!"));
		choices = do_menu("ALERT_YES_NO", string("Delete an older move to make room for ") + move + string("?"));
		if (choices[choices.size() - 1] == 1 || choices[choices.size() - 1] == -1) {
			choices = do_menu("ALERT_YES_NO", string("Abandon learning ") + move + string("?"));
			if (choices[choices.size() - 1] == 1 || choices[choices.size() - 1] == -1) {
				return learn_move(m, move);
			}
			return false;
		}
		string movelist = "";
		string followuplist = "";
		for (unsigned i = 0; i < 4; i++) {
			movelist = movelist + m.moves[i] + "|";
			followuplist = followuplist + "FORGET_INFO_CANCEL:" + m.moves[i] + "|";
		}
		movelist = movelist + move;
		followuplist = followuplist + "ABANDON_INFO_CANCEL:" + move;
		choices = do_menu("MOVE_FORGET", "", movelist, followuplist);
		if (choices[choices.size() - 1] == -1) {
			choices = do_menu("ALERT_YES_NO", string("Abandon learning ") + move + string("?"));
			if (choices[choices.size() - 1] == 1 || choices[choices.size() - 1] == -1) {
				return learn_move(m, move);
			}
			return false;
		}
		else if (choices[choices.size() - 1] == 0) {
			if (choices[choices.size() - 2] != 4) {
				string oldmove = m.moves[choices[choices.size() - 2]];
				create_move(m, move, choices[choices.size() - 2]);
				do_menu("ALERT", string("1, 2 and... Poof!"));
				do_menu("ALERT", get_nickname(m) + string(" forgot ") + oldmove + string("!"));
				do_menu("ALERT", string("And..."));
				do_menu("ALERT", get_nickname(m) + string(" learned ") + move + string("!"));
				return true;
			}
			else {
				choices = do_menu("ALERT_YES_NO", string("Abandon learning ") + move + string("?"));
				if (choices[choices.size() - 1] == 1 || choices[choices.size() - 1] == -1) {
					return learn_move(m, move);
				}
				return false;
			}
		}
	}
	return false;
}

bool engine::create_move(mon& m, string move, int index, bool overrule) {
	if (!moves[move].defined) {
		m.moves[index] = "";
		return false;
	}
	if (!overrule) {
		for (unsigned i = 0; i < 4; ++i) {
			if (m.moves[i] == move) {
				return false;
			}
		}
	}
	m.moves[index] = move;
	m.pp[index] = moves[m.moves[index]].pp;
	m.max_pp[index] = m.pp[index];
	return true;
}

void engine::clear_queue(mon& m) {
	while (m.queue.size() > 0) {
		for (int i = 0; i < 4; ++i) {
			if (m.moves[i] != "") {
				if (m.queue[0] == m.moves[i]) {
					m.pp[i]++;
				}
			}
		}
		m.queue.erase(m.queue.begin());
	}
}

double engine::random(double min, double max) {
	// min range is inclusive, max is exclusive.
	double delta = max - min;
	double t = double(rand() % 10000) / double(10000.0);
	return min + t * delta;
}

void engine::heal_damage(mon& m, int heal_amount) {
	if (heal_amount < 0) {
		// TODO:  Error logic here
		return;
	}
	m.curr_hp = m.curr_hp + heal_amount;
	if (m.curr_hp > get_stat(m, HP))
		m.curr_hp = get_stat(m, HP);
	resize_hp_bars(m);
	if (m.hud_index)
		rebuild_battle_hud(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected]);
}

void engine::deal_damage(mon& m, int damage_amount) {
	if (damage_amount <= 0) {
		// TODO:  Error logic here
		return;
	}
	m.curr_hp = m.curr_hp - damage_amount;
	if (m.curr_hp < 0)
		m.curr_hp = 0;
	resize_hp_bars(m);
	if (m.hud_index)
		rebuild_battle_hud(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected]);
}

bool engine::status_immunity(mon& m, string move) {
	for (unsigned i = 0; i < moves[move].special.size(); ++i) {
		if (moves[move].special[i].find(string("STATUS_IMMUNITY")) != -1) {
			if (get_type_1(m) != "" && moves[move].special[i].find(get_type_1(m)) != -1)
				return true;
			if (get_type_2(m) != "" && moves[move].special[i].find(get_type_2(m)) != -1)
				return true;
		}
	}
	return false;
}

bool engine::remove_status_wildcard(mon& m, string s, bool all) {
	bool success = false;
	s.erase(s.find("*"), s.length());
	while (m.status.size() > 0 && m.status[0].find(s) == 0) {
		m.status.erase(m.status.begin());
		if (!all)
			return true;
		success = true;
	}
	for (unsigned i = 1; i < m.status.size(); ++i) {
		if (m.status[i].find(s) == 0) {
			m.status.erase(m.status.begin() + i);
			if (!all)
				return true;
			success = true;
			i--;
		}
	}
	return success;
}

bool engine::remove_status(mon& m, string s, bool all) {
	bool success = false;
	if (s.find("*") != -1)
		return remove_status_wildcard(m, s, all);
	while (m.status.size() > 0 && m.status[0] == s) {
		m.status.erase(m.status.begin());
		if (!all)
			return true;
		success = true;
	}
	for (unsigned i = 1; i < m.status.size(); ++i) {
		if (m.status[i] == s) {
			m.status.erase(m.status.begin() + i);
			if (!all)
				return true;
			success = true;
			i--;
		}
	}
	return success;
}

bool engine::apply_status(mon& m, string s, bool skip_chance, bool silent) {
	string s2, s3;
	int repeat = 1;
	if (s.find("x2") != -1) {
		s.erase(s.find("x2"), string("x2").length());
		repeat = 2;
	}
	s2 = "";
	s3 = "";
	if (s.find(':') != -1) {
		s2 = s;
		s3 = s;
		s2.erase(s2.find(":"), s2.length());
		s3.erase(0, s3.find(":") + 1);
	}
	else {
		s2 = s;
	}
	if (s3 == "" && status[s2].chance)
		s3 = "100";
	if (!status[s2].defined)
		return false;
	if (s2 == "POISON" && in_status(m, string("TOXIC")))
		return false;
	if (s2 == "FLEE" && in_status(m, string("TRAP")))
		return false;
	if (status[s2].singleton) {
		if (in_status(m, s2)) {
			return false;
		}
	}
	if (!skip_chance) {
		if (status[s2].chance) {
			double c = double(stoi(s3));
			if (random(0.0, 100.0) > c) {
				return false;
			}
		}
	}
	if (!status[s2].specialcase) {
		if (s2 == "SLEEP") {
			int max;
			if (s3 == "") {
				max = int(random(1.0, 8.0));
			}
			else {
				max = stoi(s3);
			}
			for (int i = 0; i < max; ++i) {
				m.status.push_back(s2);
			}
		}
		else if (s2 == "CONFUSE") {
			int max = int(random(2.0, 5.0));
			for (int i = 0; i < max; ++i) {
				m.status.push_back(s2);
			}
		}
		else {
			string s4 = "";
			if (m.wild)
				s4 = string("Wild ");
			else if (m.enemy)
				s4 = string("Enemy ");
			s4 = s4 + get_nickname(m) + string("'s");
			if (s.find("ATTACK") != -1)
				s4 = s4 + " ATTACK ";
			else if (s.find("DEFENSE") != -1)
				s4 = s4 + " DEFENSE ";
			else if (s.find("SPEED") != -1)
				s4 = s4 + " SPEED ";
			else if (s.find("SPECIAL") != -1)
				s4 = s4 + " SPECIAL ";
			else if (s.find("ACCURACY") != -1)
				s4 = s4 + " ACCURACY ";
			else if (s.find("EVASION") != -1)
				s4 = s4 + " EVASION ";
			if (repeat > 1)
				s4 = s4 + "greatly ";
			if (s.find("UP") != -1)
				s4 = s4 + "rose!";
			if (s.find("DOWN") != -1)
				s4 = s4 + "fell!";
			if (s4.find("rose") != -1 || s4.find("fell") != -1) {
				if (in_status(m, string("LOCK_STATS"))) {
					if (!silent)
						do_alert(string("But, it failed!"));
					return false;
				}
				else {
					if (!silent)
						do_alert(s4);
				}
			}
			for (int i = 0; i < repeat; ++i)
				m.status.push_back(s2);
		}
		if (s2 == "TOXIC") {
			remove_status(m, string("POISON"), true);
			m.turn_count = 1;
		}
	}
	else {
		int value = 1;
		if (s3[0] - '0' >= 0 && s3[0] - '0' <= 9)
			value = stoi(s3);
		if (s2 == "CLEAR_STATUS") {
			remove_status(m, s3, true);
		}
		else if (s2 == "HALF_HP") {
			deal_damage(m, m.curr_hp / 2);
		}
		else if (s2 == "KO") {
			deal_damage(m, m.curr_hp);
		}
		else if (s2 == "DISABLE") {
			m.disabled_move = int(random(0.0, 4.0));
			while (!moves[m.moves[m.disabled_move]].defined)
				m.disabled_move = int(random(0.0, 4.0));
			int max = int(random(2.0, 7.0));
			for (int i = 0; i < max; ++i) {
				m.status.push_back(s2);
			}
			if (!silent)
				do_alert(string("Disabled ") + m.moves[m.disabled_move] + string("!"));
		}
		else if (s2 == "HEAL") {
			heal_damage(m, int(double(get_stat(m, HP)) * (double(value) / 100.0)));
		}
		else if (s2 == "VAMPIRE") {
			heal_damage(m, max(int(double(m.last_damage) * (double(value) / 100.0)), 1));
		}
		else if (s2 == "SELF_LEVEL") {
		}
		else if (s2 == "EXACT_DAMAGE") {
			deal_damage(m, value);
		}
		else if (s2 == "SUBSTITUTE") {
			if (get_stat(m, HP) / 4 >= m.curr_hp) {
				if (!silent)
					do_alert("But, it failed!");
			}
			else {
				int dam = get_stat(m, HP) / 4;
				deal_damage(m, dam);
				if (!silent)
					do_alert(get_nickname(m) + string(" created a SUBSTITUTE!"));
				m.stored_hp = m.curr_hp;
				m.curr_hp = 0;
				heal_damage(m, dam + 1);
				m.status.push_back(s2);
			}
		}
		else if (s2.find("TYPE") == 0) {
			m.status.push_back(s2 + string(":") + s3);
		}
		else {
			for (int i = 0; i < value; ++i) {
				m.status.push_back(s2);
			}
		}
	}
	return true;
}

void engine::use_status(mon& self, mon& other) {
	for (unsigned i = 0; i < self.status.size(); ++i) {
		if (self.status[i] == "POISON") {
			// TODO: Animation goes here
			if (self.wild)
				do_alert(string("The wild ") + get_nickname(self) + string(" is hurt by the poison!"));
			else
				do_alert(get_nickname(self) + string(" is hurt by the poison!"));
			deal_damage(self, int(max(double(get_stat(self, HP)) / 16.0, 1.0)));
		}
		else if (self.status[i] == "BURN") {
			// TODO: Animation goes here
			if (self.wild)
				do_alert(string("The wild ") + get_nickname(self) + string(" is hurt by its burn!"));
			else
				do_alert(get_nickname(self) + string(" is hurt by its burn!"));
			deal_damage(self, int(max(double(get_stat(self, HP)) / 16.0, 1.0)));
		}
		else if (self.status[i] == "SEED") {
			// TODO: Animation goes here
			if (self.wild)
				do_alert(string("The wild ") + get_nickname(self) + string("'s health was sapped by Leech Seed!"));
			else
				do_alert(get_nickname(self) + string("'s health was sapped by Leech Seed!"));
			deal_damage(self, int(max(double(get_stat(self, HP)) / 16.0, 1.0)));
			heal_damage(other, int(max(double(get_stat(self, HP)) / 16.0, 1.0)));
		}
		else if (self.status[i] == "TOXIC") {
			// TODO: Animation goes here
			if (self.wild)
				do_alert(string("The wild ") + get_nickname(self) + string(" is hurt by the poison!"));
			else
				do_alert(get_nickname(self) + string(" is hurt by the poison!"));
			deal_damage(self, int(self.turn_count * max(double(get_stat(self, HP)) / 16.0, 1.0)));
		}
	}
}

bool engine::use_move(mon& attacker, mon& defender, string move, bool skip_accuracy_check) {
	double pow = stoi(moves[move].pow);
	bool crit;
	int repeat = 1;
	bool success = false;
	bool miss = false;
	bool self_on_miss_only = false;
	string s2, s3;
	if (in_status(attacker, string("FLEE")) || in_status(defender, string("FLEE")))
		return false;
	if (in_status(attacker, string("FLINCH"))) {
		do_alert(get_nickname(attacker) + string(" flinched!"));
		remove_status(attacker, string("FLINCH"), true);
		if (has_move(attacker, attacker.queue[0])) {
			attacker.queue.clear();
		}
		return false;
	}
	if (!skip_accuracy_check && in_status(attacker, string("CONFUSE"))) {
		if (has_move(attacker, attacker.queue[0])) {
			remove_status(attacker, string("CONFUSE"));
		}
		if (!in_status(attacker, string("CONFUSE"))) {
			do_alert(get_nickname(attacker) + string(" is no longer confused!"));
		}
		else if (has_move(attacker, attacker.queue[0])) {
			do_alert(get_nickname(attacker) + string(" is confused!"));
			if (int(random(0.0, 100.0)) > 50) {
				attacker.queue.clear();
				return use_move(attacker, attacker, string("CONFUSED"), true);
			}
		}
	}
	if (moves[move].desc == "") {
		do_alert(get_nickname(attacker) + string(" used ") + move + string("!"));
	}
	else if (moves[move].desc != "{NONE}") {
		string temp = moves[move].desc;
		while (temp.find("{NAME}") != -1) {
			temp.insert(temp.find("{NAME}"), get_nickname(attacker));
			temp.erase(temp.find("{NAME}"), string("{NAME}").length());
		}
		while (temp.find("{BREAK}") != -1) {
			string temp2;
			temp2 = temp;
			temp2.erase(temp2.find("{BREAK}"), temp2.length());
			temp.erase(0, temp.find("{BREAK}") + string("{BREAK}").length());
			do_alert(temp2);
		}
		do_alert(temp);
	}
	for (unsigned i = 0; i < 4; ++i) {
		if (attacker.moves[i] == move) {
			attacker.pp[i]--;
			// TODO: Fix bug where Mirror Move consumes PP when copying a known move.
		}
	}
	if (in_status(attacker, string("DISABLE")) && attacker.moves[attacker.disabled_move] == move) {
		do_alert("But, it failed!");
		attacker.queue.clear();
		return false;
	}
	attacker.last_move = move;
	for (unsigned i = 0; i < moves[move].special.size(); ++i) {
		if (moves[move].special[i] == "UNAVOIDABLE")
			skip_accuracy_check = true;
		else if (moves[move].special[i] == "COUNTER") {
			if (attacker.last_hit_physical == 0) {
				miss = true;
			}
			else {
				attacker.last_damage = min(attacker.last_hit_physical * 2, defender.curr_hp);
				deal_damage(defender, attacker.last_damage);
			}
		}
		else if (moves[move].special[i] == "SELF_ON_MISS_ONLY") {
			self_on_miss_only = true;
		}
		else if (moves[move].special[i] == "ENEMY_LAST") {
			if (defender.last_move != "" && !moves[defender.last_move].queue_only) {
				attacker.queue.erase(attacker.queue.begin());
				return use_move(attacker, defender, defender.last_move, skip_accuracy_check);
			}
			else {
				do_alert("But, it failed!");
				attacker.queue.erase(attacker.queue.begin());
				return false;
			}
		}
		else if (moves[move].special[i] == "RANDOM") {
			std::map<string, power>::iterator it = moves.begin();
			int i = int(random(1, moves.size() - 1));
			while (i > 0) {
				i--;
				it++;
			}
			while (it->second.queue_only) {
				it++;
			}
			if (it == moves.end())
				it--;
			while (it->second.queue_only) {
				it--;
			}
			use_move(attacker, defender, it->second.name);
		}
		else if (moves[move].special[i] == "PAYDAY") {
			mc.extra_winnings += 5 * attacker.level;
		}
		else if (moves[move].special[i] == "CONVERSION") {
			remove_status(attacker, string("TYPE*"), true);
			apply_status(attacker, string("TYPE1:") + get_type_1(defender));
			apply_status(attacker, string("TYPE2:") + get_type_2(defender));
		}
		else if (moves[move].special[i] == "SLEEPING_TARGET_ONLY" && !in_status(defender, string("SLEEP"))) {
			miss = true;
			pow = 0;
		}
		else if (moves[move].special[i] == "TRANSFORM") {
			bool has_nontransform_move = false;
			for (unsigned j = 0; j < 4; ++j) {
				if (moves[defender.moves[j]].defined) {
					bool is_transform = false;
					for (unsigned k = 0; k < moves[defender.moves[j]].special.size(); ++k) {
						if (moves[defender.moves[j]].special[k] == "TRANSFORM")
							is_transform = true;
					}
					if (!is_transform)
						has_nontransform_move = true;
				}
			}
			if (has_nontransform_move) {
				mon* temp = new mon;
				temp->original = attacker.original;
				attacker.original = temp;
				*(attacker.original) = attacker;
				attacker.original->enemy = attacker.enemy;
				attacker = defender;
				attacker.level = attacker.original->level;
				attacker.nickname = get_nickname(*attacker.original);
				attacker.exp = attacker.original->exp;
				attacker.original->exp_bar_index = attacker.exp_bar_index;
				for (unsigned im = 0; i < SIZE; ++i) {
					attacker.IV[i] = attacker.original->IV[i];
					attacker.EV[i] = attacker.original->EV[i];
				}
				for (unsigned i = 0; i < 4; ++i) {
					attacker.pp[i] = 5;
				}
				attacker.curr_hp = min(get_stat(attacker, HP), attacker.original->curr_hp);
				attacker.queue.clear();
				attacker.enemy = attacker.original->enemy;
				attacker.wild = attacker.original->wild;
				if (attacker.enemy) {
					rebuild_battle_hud(defender, attacker);
				}
				else {
					rebuild_battle_hud(attacker, defender);
				}
			}
			else {
				do_alert("But, it failed!");
			}
		}
	}
	if (miss || (!skip_accuracy_check && ((moves[move].acc < int(random(0.0, 100.0) * get_evasion_modifier(defender) / get_accuracy_modifier(attacker)))) || in_status(defender, string("UNTARGETABLE")))) {
		miss = true;
		if (pow == 0) {
			do_alert(string("But, it failed!"));
		}
		else {
			do_alert(get_nickname(attacker) + string("'s attack missed!"));
		}
		if (self_on_miss_only) {
			for (unsigned j = 0; j < moves[move].self.size(); ++j) {
				success = apply_status(attacker, moves[move].self[j]) || success;
			}
		}
	}
	if (pow != 0.0) {
		if (moves[move].pow.find(string("x2-5")) != -1) {
			repeat = weightedrand() + 2;
		}
		else if (moves[move].pow.find(string("x2")) != -1) {
			repeat = 2;
		}
	}
	if (!miss) {
		for (int i = 0; i < repeat; ++i) {
			double mul;
			bool non_zero;
			int dam = min(damage(attacker, defender, move, crit, mul, non_zero), defender.curr_hp);
			deal_damage(defender, dam);
			if (moves[move].defense == SPECIAL) {
				defender.last_hit_special = dam;
				defender.last_hit_physical = 0;
			}
			else if (moves[move].defense == DEFENSE) {
				defender.last_hit_special = 0;
				defender.last_hit_physical = dam;
			}
			attacker.last_damage = dam;
			if (dam > 0 && in_status(defender, string("RAGE")) && !is_KO(defender)) {
				apply_status(defender, string("ATTACK_UPx2"));
				apply_status(defender, string("DEFENSE_UPx2"));
				apply_status(defender, string("SPECIAL_UPx2"));
			}
			if (non_zero) {
				if (mul == 4.0)
					do_alert("It's extremely effective!");
				else if (mul == 2.0)
					do_alert("It's super effective!");
				else if (mul == 0.0)
					do_alert(string("It doesn't affect ") + get_nickname(defender) + string("!"));
				else if (mul == 0.5)
					do_alert("It's not very effective...");
				else if (mul == 0.25)
					do_alert("It barely had an effect...");
				if (dam > 0 && crit)
					do_alert(string("A critical hit!"));
			}
			if (!status_immunity(defender, move)) {
				for (unsigned j = 0; j < moves[move].target.size(); ++j) {
					if (moves[move].target[j].find("EXACT_DAMAGE") == 0)
						continue;
					if (moves[move].target[j].find("HALF_HP") == 0)
						continue;
					if (moves[move].target[j].find("KO") == 0 && mul == 0.0) {
						if (defender.wild)
							do_alert(string("It didn't have any effect on wild ") + get_nickname(defender) + string("!"));
						else if (defender.enemy)
							do_alert(string("It didn't have any effect on enemy ") + get_nickname(defender) + string("!"));
						else
							do_alert(string("It didn't have any effect on ") + get_nickname(defender) + string("!"));
						continue;
					}
					success = apply_status(defender, moves[move].target[j]) || success;
					// TODO:  Status announcement
				}
			}
			if (!self_on_miss_only) {
				for (unsigned j = 0; j < moves[move].self.size(); ++j) {
					if (moves[move].self[j].find("RECOIL") != -1) {
						s2 = moves[move].self[j];
						s3 = moves[move].self[j];
						s2.erase(s2.find(":"), s2.length());
						s3.erase(0, s3.find(":") + 1);
						deal_damage(attacker, int(max(double(dam)*(double(stoi(s3)) / 100.0), 1.0)));
					}
					else {
						success = apply_status(attacker, moves[move].self[j]) || success;
					}
				}
			}
		}
		for (unsigned i = 0; i < moves[move].additional.size(); ++i) {
			success = use_move(attacker, defender, moves[move].additional[i], true) || success;
		}
	}
	for (unsigned i = 0; i < moves[move].queue.size(); ++i) {
		if (moves[move].queue[i].find("x0-3") == -1)
			attacker.queue.push_back(moves[move].queue[i]);
		else {
			string temp = moves[move].queue[i];
			temp.erase(temp.find("x0-3"), 4);
			int r = weightedrand();
			for (int j = 0; j < r; ++j)
				attacker.queue.push_back(temp);
		}
	}
	if (!miss && repeat != 1) {
		do_alert(string("Hit ") + to_string(repeat) + string(" times!"));
	}
	if (attacker.queue.size() > 0) {
		if (attacker.queue[0] == move) {
			attacker.queue.erase(attacker.queue.begin());
		}
	}
	if (miss && in_special(move, string("CLEAR_QUEUE_ON_FAIL"))) {
		clear_queue(attacker);
	}
	return success;
}

bool engine::can_evolve(string& ID, string& effect) {
	for (unsigned i = 0; i < all_mon[ID].evolution.size(); ++i) {
		if (all_mon[ID].evolution[i].second == effect)
			return true;
	}
	return false;
}

bool engine::is_KO(mon& m) {
	if (m.curr_hp <= 0) {
		if (in_status(m, "SUBSTITUTE")) {
			m.curr_hp = 0;
			do_alert("The SUBSTITUTE broke!");
			heal_damage(m, m.stored_hp);
			remove_status(m, string("SUBSTITUTE"));
			return false;
		}
		return true;
	}
	return false;
}

bool engine::team_KO() {
	bool team_exists = false;
	for (unsigned i = 0; i < 6; ++i) {
		team_exists = team_exists | mc.team[i].defined;
	}
	if (!team_exists) {
		return false;
	}
	for (unsigned i = 0; i < 6; ++i) {
		if (mc.team[i].defined && mc.team[i].curr_hp > 0)
			return false;
	}
	return true;
}

bool engine::has_move(mon& m, string& move) {
	for (unsigned i = 0; i < 4; ++i) {
		if (m.moves[i] == move) {
			return true;
		}
	}
	return false;
}

void engine::KO_move_clear(mon& m) {
	bool clear = true;
	if (is_KO(m)) {
	}
	else if (m.queue.size() > 0 && has_move(m, m.queue[0])) {
	}
	else if (m.queue.size() > 0 && (m.queue[0] == "STRUGGLE" || m.queue[0] == "CONFUSED" || m.queue[0] == "")) {
	}
	else {
		clear = false;
	}
	if (clear) {
		m.queue.clear();
	}
}

void engine::do_turn_inner(mon& m1, mon& m2) {
	int temp;
	m1.turn_count++;
	m2.turn_count++;
	if (in_status(m1, string("SLEEP"))) {
		remove_status(m1, string("SLEEP"));
		if (in_status(m1, string("SLEEP"))) {
			do_alert(get_nickname(m1) + string(" is fast asleep."));
			if (has_move(m1, m1.queue[0]) || m1.queue[0] == "")
				m1.queue.clear();
			// TODO:  Sleep animation.
		}
		else {
			do_alert(get_nickname(m1) + string(" woke up!"));
			if (has_move(m1, m1.queue[0]) || m1.queue[0] == "")
				m1.queue.clear();
		}
	}
	else if (in_status(m1, string("FREEZE"))) {
		if (0.2 > random(0.0, 1.0)) {
			remove_status(m1, string("FREEZE"));
			do_alert(get_nickname(m1) + string(" thawed out!"));
			if (has_move(m1, m1.queue[0]) || m1.queue[0] == "")
				m1.queue.clear();
		}
		else {
			do_alert(get_nickname(m1) + string(" is frozen solid!"));
			if (has_move(m1, m1.queue[0]) || m1.queue[0] == "")
				m1.queue.clear();
			// TODO:  Freeze animation
		}
	}
	else if (in_status(m1, string("PARALYZE")) && (0.25 > random(0.0, 1.0))) {
		do_alert(get_nickname(m1) + string(" is paralyzed! It can't move!"));
		if (has_move(m1, m1.queue[0]) || m1.queue[0] == "")
			m1.queue.clear();
		// TODO:  Paralyze animation
	}
	else if (m1.queue[0] != "") {
		if (in_status(m1, string("UNTARGETABLE")))
			remove_status(m1, string("UNTARGETABLE"));
		temp = m2.curr_hp;
		use_move(m1, m2, m1.queue[0]);
		m2.damage_dealt.push_back(temp - m2.curr_hp);
		if (is_KO(m2)) {
			KO_move_clear(m1);
			KO_move_clear(m2);
			// TODO:  Return value
			return;
		}
		use_status(m1, m2);
		if (is_KO(m1)) {
			KO_move_clear(m1);
			KO_move_clear(m2);
			// TODO:  Return value
			return;
		}
		if (in_status(m1, string("DISABLE"))) {
			remove_status(m1, string("DISABLE"));
			if (!in_status(m1, string("DISABLE"))) {
				do_alert(moves[m1.moves[m1.disabled_move]].name + string(" is no longer disabled!"));
			}
		}
		if (in_status(m1, string("LOCK_STATS"))) {
			remove_status(m1, string("LOCK_STATS"));
			if (!in_status(m1, string("LOCK_STATS"))) {
				do_alert(string("Stats are no longer locked!"));
			}
		}
	}
	else {
		m1.queue.erase(m1.queue.begin());
	}
	if (in_status(m2, string("SLEEP"))) {
		remove_status(m2, string("SLEEP"));
		if (in_status(m2, string("SLEEP"))) {
			do_alert(get_nickname(m2) + string(" is fast asleep."));
			if (has_move(m2, m2.queue[0]) || m2.queue[0] == "")
				m2.queue.clear();
			// TODO:  Sleep animation
		}
		else {
			do_alert(get_nickname(m2) + string(" woke up!"));
			if (has_move(m2, m2.queue[0]) || m2.queue[0] == "")
				m2.queue.clear();
		}
	}
	else if (in_status(m2, string("FREEZE"))) {
		if (0.2 > random(0.0, 1.0)) {
			remove_status(m2, string("FREEZE"));
			do_alert(get_nickname(m2) + string(" thawed out!"));
			if (has_move(m2, m2.queue[0]) || m2.queue[0] == "")
				m2.queue.clear();
		}
		else {
			do_alert(get_nickname(m2) + string(" is frozen solid!"));
			if (has_move(m2, m2.queue[0]) || m2.queue[0] == "")
				m2.queue.clear();
			// TODO:  Freeze animation
		}
	}
	else if (in_status(m2, string("PARALYZE")) && (0.25 > random(0.0, 1.0))) {
		do_alert(get_nickname(m2) + string(" is paralyzed! It can't move!"));
		if (has_move(m2, m2.queue[0]) || m2.queue[0] == "")
			m2.queue.clear();
		// TODO:  Paralyze animation
	}
	else if (m2.queue[0] != "") {
		if (in_status(m2, string("UNTARGETABLE")))
			remove_status(m2, string("UNTARGETABLE"));
		temp = m1.curr_hp;
		use_move(m2, m1, m2.queue[0]);
		m1.damage_dealt.push_back(temp - m1.curr_hp);
		if (is_KO(m1)) {
			KO_move_clear(m1);
			KO_move_clear(m2);
			// TODO:  Return value
			return;
		}
		use_status(m2, m1);
		if (is_KO(m2)) {
			KO_move_clear(m1);
			KO_move_clear(m2);
			// TODO:  Return value
			return;
		}
		if (in_status(m2, string("DISABLE"))) {
			remove_status(m2, string("DISABLE"));
			if (!in_status(m2, string("DISABLE"))) {
				do_alert(moves[m2.moves[m2.disabled_move]].name + string(" is no longer disabled!"));
			}
		}
		if (in_status(m2, string("LOCK_STATS"))) {
			remove_status(m2, string("LOCK_STATS"));
			if (!in_status(m2, string("LOCK_STATS"))) {
				do_alert(string("Stats are no longer locked!"));
			}
		}
	}
	else {
		m2.queue.erase(m2.queue.begin());
	}
}

void engine::clear_volatile(mon& m) {
	mon* temp;
	while (m.original) {
		m.original->curr_hp = m.curr_hp;
		m.original->status = m.status;
		m.original->enemy = m.enemy;
		m.original->wild = m.wild;
		m = *(m.original);
		m.enemy = m.original->enemy;
		m.wild = m.original->wild;
		temp = m.original;
		m.original = m.original->original;
		delete temp;
	}
	map<string, status_effect>::iterator it;
	if (m.defined) {
		for (unsigned i = 0; i < m.status.size(); ++i) {
			string s;
			s = m.status[i];
			if (s.find(":") != -1) {
				s.erase(s.find(":"), s.length());
			}
			if (status[s].defined && !status[s].nonvolatile) {
				remove_status(m, m.status[i], true);
				i--;
			}
		}
	}
}

void engine::team_clear_volatile() {
	for (unsigned i = 0; i < 6; ++i) {
		clear_volatile(mc.team[i]);
	}
}

float engine::get_hp_percent(mon& m) {
	return float(m.curr_hp) / float(get_stat(m, HP));
}

bool engine::is_valid_move(mon& m, unsigned i) {
	if (!moves[m.moves[i]].defined)
		return false;
	if (m.pp[i] <= 0)
		return false;
	if (in_status(m, string("DISABLE")) && i == m.disabled_move)
		return false;
	return true;
}

double engine::get_smart_damage(mon& attacker, mon& defender, string move, trainer& t, int future) {
	bool crit = false;
	bool skip_accuracy_check = false;
	double mul = 0.0;
	bool non_zero;
	int buff = 0;
	if (in_move_special(move, string("SLEEPING_TARGET_ONLY")) && !in_status(defender, string("SLEEP")))
		return 0.0;
	for (unsigned j = 0; j < attacker.status.size(); ++j) {
		if (attacker.status[j] == this->get_buff_name(SPEED)) {
			buff++;
		}
		if (attacker.status[j] == this->get_debuff_name(SPEED)) {
			buff--;
		}
	}
	double chance = get_stat_modifier(buff) * all_mon[attacker.number].stats[SPEED] / 4.0;
	if (in_status(attacker, string("FOCUS")))
		chance *= 4.0;
	chance *= moves[move].crit_chance;
	chance = chance / 256.0;
	double low = damage(attacker, defender, move, crit, mul, non_zero, true, false, true);
	double high = damage(attacker, defender, move, crit, mul, non_zero, false, true, true);
	double pow = low + chance * (high - low);
	pow = pow * 0.85;
	for (unsigned i = 0; i < moves[move].target.size(); ++i) {
		if (moves[move].target[i] == "KO") {
			pow = defender.curr_hp;
		}
	}
	if (moves[move].pow.find(string("x2-5")) != -1) {
		if (!in_status(defender, string("RAGE")))
			pow *= 3.0;
	}
	else if (moves[move].pow.find(string("x2")) != -1) {
		if (!in_status(defender, string("RAGE")))
			pow *= 2.0;
	}
	int turn_count = 1;
	if (in_move_target(move, string("FLINCH")) && in_move_target(move, string("TRAP"))) {
		for (unsigned j = 0; j < moves[move].queue.size(); ++j) {
			turn_count++;
		}
		if (get_stat(attacker, SPEED) < get_stat(defender, SPEED)) {
			turn_count--;
		}
	}
	pow *= double(turn_count);
	for (unsigned j = 0; j < moves[move].special.size(); ++j) {
		if (moves[move].special[j] == "UNAVOIDABLE") {
			skip_accuracy_check = true;
		}
	}
	if (!skip_accuracy_check && !t.skip_accuracy_check) {
		pow *= get_accuracy_modifier(attacker);
		pow *= double(moves[move].acc) / 100.0;
	}
	if (in_status(defender, string("POISON")) && !in_status(attacker, string("BURN")) && !in_status(attacker, string("POISON")) && !in_status(attacker, string("TOXIC"))) {
		pow += double(get_stat(defender, HP)*turn_count) / 16.0;
	}
	if (in_status(defender, string("BURN")) && !in_status(attacker, string("BURN")) && !in_status(attacker, string("POISON")) && !in_status(attacker, string("TOXIC"))) {
		pow += double(get_stat(defender, HP)*turn_count) / 16.0;
	}
	else if (in_status(defender, string("TOXIC")) && !in_status(attacker, string("TOXIC"))) {
		double scale = 1.0;
		double toxic_damage = 0.0;
		if (in_status(defender, string("SLEEP")))
			scale *= 1.5;
		if (in_status(defender, string("CONFUSE")))
			scale *= 1.5;
		for (int i = 1; i <= turn_count * 2; ++i) {
			toxic_damage += scale * double(get_stat(defender, HP)*(future + defender.turn_count + i)) / 16.0;
		}
		toxic_damage /= 2.0;
		pow += toxic_damage;
	}
	return pow;
}

double engine::get_smart_healing(mon& attacker, mon& defender, string move, trainer& t, int future) {
	bool crit = false;
	bool skip_accuracy_check = false;
	double mul = 0.0;
	bool non_zero;
	int buff = 0;
	double ret = 0.0;
	if (in_move_special(move, string("SLEEPING_TARGET_ONLY")) && !in_status(defender, string("SLEEP")))
		return 0.0;
	for (unsigned j = 0; j < attacker.status.size(); ++j) {
		if (attacker.status[j] == this->get_buff_name(SPEED)) {
			buff++;
		}
		if (attacker.status[j] == this->get_debuff_name(SPEED)) {
			buff--;
		}
	}
	double chance = get_stat_modifier(buff) * all_mon[attacker.number].stats[SPEED] / 4.0;
	if (in_status(attacker, string("FOCUS")))
		chance *= 4.0;
	chance *= moves[move].crit_chance;
	chance = chance / 256.0;
	double low = damage(attacker, defender, move, crit, mul, non_zero, true, false, true);
	double high = damage(attacker, defender, move, crit, mul, non_zero, false, true, true);
	double pow = low + chance * (high - low);
	pow = pow * 0.85;
	if (moves[move].pow.find(string("x2-5")) != -1) {
		if (!in_status(defender, string("RAGE")))
			pow *= 3.0;
	}
	else if (moves[move].pow.find(string("x2")) != -1) {
		if (!in_status(defender, string("RAGE")))
			pow *= 2.0;
	}
	int turn_count = 1;
	if (in_move_target(move, string("FLINCH")) && in_move_target(move, string("TRAP"))) {
		for (unsigned j = 0; j < moves[move].queue.size(); ++j) {
			turn_count++;
		}
		if (get_stat(attacker, SPEED) < get_stat(defender, SPEED)) {
			turn_count--;
		}
	}
	pow *= double(turn_count);
	for (unsigned j = 0; j < moves[move].special.size(); ++j) {
		if (moves[move].special[j] == "UNAVOIDABLE") {
			skip_accuracy_check = true;
		}
	}
	if (!skip_accuracy_check && !t.skip_accuracy_check) {
		pow *= get_accuracy_modifier(attacker);
		pow *= double(moves[move].acc) / 100.0;
	}
	for (unsigned j = 0; j < moves[move].self.size(); ++j) {
		if (moves[move].self[j].find("VAMPIRE") != -1) {
			string to_parse = moves[move].self[j];
			mul = 1.0;
			if (to_parse.find(":") != -1) {
				to_parse.erase(0, to_parse.find(":") + 1);
				mul = double(stoi(to_parse)) / 100.0;
			}
			ret += min(pow*mul, double(get_stat(attacker, HP) - attacker.curr_hp));
		}
		if (moves[move].self[j].find("HEAL") != -1) {
			string to_parse = moves[move].self[j];
			mul = 1.0;
			if (to_parse.find(":") != -1) {
				to_parse.erase(0, to_parse.find(":") + 1);
				mul = double(stoi(to_parse)) / 100.0;
			}
			ret += min(double(get_stat(attacker, HP))*mul, double(get_stat(attacker, HP) - attacker.curr_hp));
		}
	}
	if (in_status(defender, string("SEED")) && !in_status(attacker, string("BURN")) && !in_status(attacker, string("POISON")) && !in_status(attacker, string("TOXIC"))) {
		ret += double(get_stat(defender, HP)*turn_count) / 16.0;
	}
	return ret;
}

int engine::get_smart_move(mon& attacker, mon& defender, trainer& t, bool skip_recurse, int hp_offset, int& self_turns_to_live, int& opponent_turns_to_live, bool no_depth, double& future_damage) {
	int a = 0, b = 0;
	double dam;
	int enemy_move = -1;
	double enemy_damage = 0.0;
	double self_damage = 0.0;
	double enemy_heal = 0.0;
	double self_heal = 0.0;
	double magnitude = -10000000.0;
	double pow = 0.0;
	bool crit = false;
	double mul = 0.0;
	double chance = 0.0;
	int turns_to_live = -1;
	int holder_turns_to_live = -1;
	int ret = -1;
	future_damage = 0.0;
	if (!skip_recurse) {
		future_damage = 0.0;
		enemy_move = get_smart_move(defender, attacker, t, true, 0, a, b, false, future_damage);
		if (enemy_move >= 0) {
			enemy_damage = get_smart_damage(defender, attacker, defender.moves[enemy_move], t);
			enemy_heal = get_smart_healing(defender, attacker, defender.moves[enemy_move], t);
		}
		else
			enemy_damage = get_smart_damage(defender, attacker, string("STRUGGLE"), t);
		if (future_damage > 0.0) {
			enemy_damage = future_damage;
		}
		turns_to_live = (attacker.curr_hp / max(int(enemy_damage), 1));
		if (future_damage > 0.0) {
			turns_to_live++;
		}
		if (get_stat(attacker, SPEED) > get_stat(defender, SPEED)) {
			turns_to_live += 1;
		}
		self_turns_to_live = turns_to_live;
	}
	bool delay;
	for (unsigned i = 0; i < 4; ++i) {
		if (is_valid_move(attacker, i)) {
			delay = false;
			self_damage = get_smart_damage(attacker, defender, attacker.moves[i], t);
			self_heal = get_smart_healing(attacker, defender, attacker.moves[i], t);
			if ((self_damage + self_heal) == 0.0 && moves[attacker.moves[i]].target.size() == 0) {
				for (unsigned j = 0; j < moves[attacker.moves[i]].self.size(); ++j) {
					if (moves[attacker.moves[i]].self[j] == "UNTARGETABLE") {
						self_damage = get_smart_damage(attacker, defender, moves[attacker.moves[i]].queue[0], t);
						self_heal = get_smart_healing(attacker, defender, moves[attacker.moves[i]].queue[0], t);
					}
				}
			}
			else if ((self_damage + self_heal) == 0.0 && moves[attacker.moves[i]].target.size() != 0) {
				if (!no_depth) {
					mon defendercopy;
					defendercopy = defender;
					bool applied = false;
					for (unsigned j = 0; j < moves[attacker.moves[i]].target.size(); ++j) {
						if (moves[attacker.moves[i]].target[j] != "KO") {
							applied = applied || apply_status(defendercopy, moves[attacker.moves[i]].target[j], true, true);
						}
					}
					for (unsigned j = 0; j < moves[attacker.moves[i]].special.size(); ++j) {
						if (moves[attacker.moves[i]].special[j].find(string("STATUS_IMMUNITY")) != -1) {
							if (get_type_1(defender) != "" && moves[attacker.moves[i]].special[j].find(get_type_1(defender)) != -1) {
								applied = false;
							}
							if (get_type_2(defender) != "" && moves[attacker.moves[i]].special[j].find(get_type_2(defender)) != -1) {
								applied = false;
							}
						}
					}
					if (applied) {
						int temp1, temp2;
						double temp3;
						int future_move = get_smart_move(attacker, defendercopy, t, skip_recurse, hp_offset, temp1, temp2, true, temp3);
						self_damage = get_smart_damage(attacker, defendercopy, attacker.moves[future_move], t, 1);
						self_heal = get_smart_healing(attacker, defendercopy, attacker.moves[future_move], t, 1);
						holder_turns_to_live = (attacker.curr_hp / max(int(enemy_damage - self_heal), 1));
						if (in_status(defendercopy, string("SLEEP"))) {
							holder_turns_to_live++;
						}
						if (in_status(defendercopy, string("FREEZE"))) {
							holder_turns_to_live++;
						}
						if (get_stat(attacker, SPEED) > get_stat(defendercopy, SPEED)) {
							holder_turns_to_live += 1;
						}
						delay = true;
						if (!in_status(defender, string("SLEEP")) && in_status(defendercopy, string("SLEEP"))) {
							self_damage += enemy_heal * 3.0;
							self_heal += enemy_damage * 3.0;
						}
						if (!in_status(defender, string("PARALYZE")) && in_status(defendercopy, string("PARALYZE"))) {
							double temp = min((defendercopy.curr_hp / max(int(self_damage - enemy_heal), 1)), holder_turns_to_live);
							self_damage += enemy_heal * 0.25 * temp;
							self_heal += enemy_damage * 0.25 * temp;
						}
						if (!in_status(defender, string("CONFUSE")) && in_status(defendercopy, string("CONFUSE"))) {
							self_damage += enemy_heal * 0.5;
							self_heal += enemy_damage * 0.5;
						}
						self_damage *= double(moves[attacker.moves[i]].acc) / 100.0;
						self_heal *= double(moves[attacker.moves[i]].acc) / 100.0;
						self_damage *= double(holder_turns_to_live - 1) / double(max(holder_turns_to_live, 1));
						self_heal *= double(holder_turns_to_live - 1) / double(max(holder_turns_to_live, 1));
					}
				}
			}
			if (!skip_recurse) {
				for (unsigned j = 0; j < moves[attacker.moves[i]].self.size(); ++j) {
					if (moves[attacker.moves[i]].self[j] == "KO") {
						if (turns_to_live > 1 || in_status(defender, string("SLEEP")) || in_status(defender, string("FREEZE"))) {
							self_damage = 0.0;
							self_heal = 0.0;
							break;
						}
					}
					if (moves[attacker.moves[i]].self[j] == "RAGE") {
						self_damage *= min(double(turns_to_live), 4.0);
						break;
					}
				}
				for (unsigned j = 0; j < moves[attacker.moves[i]].special.size(); ++j) {
					if (moves[attacker.moves[i]].special[j] == "FIRST") {
						if (turns_to_live <= 0) {
							self_damage *= 1000.0;
							break;
						}
					}
				}
			}
			else {
				for (unsigned j = 0; j < moves[attacker.moves[i]].self.size(); ++j) {
					if (moves[attacker.moves[i]].self[j] == "KO") {
						self_damage = 0.0;
						break;
					}
				}
			}
			if ((enemy_damage >= attacker.curr_hp) && (get_stat(attacker, SPEED) < get_stat(defender, SPEED))) {
				self_heal = 0;
			}
			if ((get_stat(attacker, SPEED) > get_stat(defender, SPEED)) && self_damage >= defender.curr_hp) {
				self_heal += enemy_damage;
			}
			dam = (self_damage + self_heal) - (enemy_damage + enemy_heal);
			dam = (-double(defender.curr_hp) / max(self_damage - enemy_heal, 1.0)) + (double(attacker.curr_hp - hp_offset) / max(enemy_damage - self_heal, 1.0));
			if (delay)
				dam -= 1.0;
			if (dam > magnitude) {
				magnitude = dam;
				ret = i;
				self_turns_to_live = (attacker.curr_hp / max(int(enemy_damage - self_heal), 1));
				opponent_turns_to_live = (defender.curr_hp / max(int(self_damage - enemy_heal), 1));
				if (delay) {
					future_damage = self_damage;
					opponent_turns_to_live++;
				}
				else {
					future_damage = 0.0;
				}
			}
		}
	}
	return ret;
}

void engine::resize_exp_bar(mon& m) {
	if (m.exp_bar_index > 0) {
		float f = float(m.exp - level_to_exp[m.level]) / float(level_to_exp[m.level + 1] - level_to_exp[m.level]);
		f = fmin(f, 1.0f);
		g.draw_list[m.exp_bar_index].width = 0.85f * f;
		g.draw_list[m.exp_bar_index].x = 0.85f * (1.0 - f);
	}
}

void engine::resize_hp_bars(mon& m) {
	if (m.hp_bar_index > 0) {
		float f = get_hp_percent(m);
		g.draw_list[m.hp_bar_index].width = 0.6f * f;
		if (f > 0.66f) {
			g.draw_list[m.hp_bar_index].filename = string("../resources/images/hp-good.bmp");
			g.draw_list[m.hp_bar_index].tex = g.tex[string("../resources/images/hp-good.bmp")];
			g.new_load = true;
		}
		else if (f > 0.33f) {
			g.draw_list[m.hp_bar_index].filename = string("../resources/images/hp-medium.bmp");
			g.draw_list[m.hp_bar_index].tex = g.tex[string("../resources/images/hp-medium.bmp")];
			g.new_load = true;
		}
		else {
			g.draw_list[m.hp_bar_index].filename = string("../resources/images/hp-bad.bmp");
			g.draw_list[m.hp_bar_index].tex = g.tex[string("../resources/images/hp-bad.bmp")];
			g.new_load = true;
		}
	}
}

vector<int> engine::remove_cancels(vector<int> v) {
	for (unsigned i = 0; i < v.size(); ++i) {
		if (v[i] == -1) {
			v.erase(v.begin() + i);
			if (v.size() > 0)
				v.erase(v.begin() + i - 1);
			i = 0;
		}
	}
	return v;
}

void engine::rebuild_battle_hud(mon& p, mon& e) {
	m.lock();
	if (p.hp_bar_index != 0 && e.hp_bar_index != 0) {
		unsigned clear_point = p.hud_index;
		g.draw_list.erase(g.draw_list.begin() + clear_point, g.draw_list.end());
		g.push_text(-0.9f, 0.8f, 1.5f, 0.1f, 0.1f, get_nickname(e));
		g.push_text(-0.6f, 0.7f, 0.5f, 0.1f, 0.1f, string("{LEVEL}") + to_string(e.level));
		g.push_text(1.0f - (float(get_nickname(p).length() * 0.1f)), -0.1f, 0.0f, 0.1f, 0.1f, get_nickname(p));
		g.push_text(0.4f, -0.2f, 0.5f, 0.1f, 0.1f, string("{LEVEL}") + to_string(p.level));
		string formatted_hp = "";
		formatted_hp += to_string(p.curr_hp) + string("/") + to_string(get_stat(p, HP));
		g.push_text(0.8f - (float(formatted_hp.length())*0.1f), -0.4f, 0.8f, 0.1f, 0.1f, formatted_hp);
		g.draw_list[p.sprite_index].filename = string("../resources/images/") + p.number + string("-back.png");
		g.draw_list[p.sprite_index].tex = g.tex[g.draw_list[p.sprite_index].filename];
		g.draw_list[e.sprite_index].filename = string("../resources/images/") + e.number + string(".png");
		g.draw_list[e.sprite_index].tex = g.tex[g.draw_list[e.sprite_index].filename];
		resize_exp_bar(p);
		resize_hp_bars(p);
		resize_hp_bars(e);
		g.new_load = true;
	}
	m.unlock();
}

bool engine::battle(trainer& t) { // trainer battle
	int i;
	int index;
	int escape_attempts = 0;
	vector<int> choices;
	double count;
	int clear_point = g.draw_list.size();
	int old_team_selected;
	g.push_box(-1.1f, -1.1f, 2.2f, 2.2f);
	g.push_box(-1.0f, -1.0f, 2.0f, 0.6f);
	mc.selected = -1;
	mc.extra_winnings = 0;
	mc.enemy_selected = 0;
	for (i = 0; i < 6; ++i) {
		if (mc.team[i].defined) {
			if (!is_KO(mc.team[i])) {
				mc.selected = i;
				break;
			}
		}
	}
	for (i = 0; i < 6; ++i) {
		mc.enemy_team[i] = t.team[i];
		mc.enemy_team[i].wild = false;
		mc.enemy_team[i].enemy = true;
		for (int j = 0; j < 6; ++j) {
			mc.enemy_team[i].fought[j] = false;
		}
	}
	mc.team[mc.selected].queue.clear();
	mc.team[mc.selected].last_move = "";
	mc.enemy_team[mc.enemy_selected].queue.clear();
	mc.enemy_team[mc.enemy_selected].last_move = "";
	unsigned enemy_trainer_sprite = g.push_quad_load(0.1f, 0.1f, 0.9f, 0.9f, string("../resources/images/") + t.image);
	mc.enemy_team[mc.enemy_selected].hp_bar_index = g.push_hp_bar(-0.7f, 0.7f, get_hp_percent(mc.enemy_team[mc.enemy_selected]));
	mc.enemy_team[mc.enemy_selected].hud_index = 0;
	unsigned player_sprite = g.push_quad_load(-1.0f, -0.422f, 0.9f, 0.9f, string("../resources/images/player-back.png"));
	do_alert(t.display_name + string(" wants to fight!"));
	g.draw_list[enemy_trainer_sprite].x = 2.0f; // TODO:  Animation
	unsigned enemy_sprite = g.push_quad_load(0.1f, 0.1f, 0.9f, 0.9f, string("../resources/images/") + mc.enemy_team[mc.enemy_selected].number + string(".png"));
	do_alert(t.display_name + string(" sent out ") + get_nickname(mc.enemy_team[mc.enemy_selected]) + string("!"));
	mc.enemy_team[mc.enemy_selected].turn_count = 1;
	mc.seen[mc.enemy_team[mc.enemy_selected].number] = true;
	do_alert(string("Go! ") + get_nickname(mc.team[mc.selected]) + string("!"));
	mc.team[mc.selected].turn_count = 1;
	g.draw_list[player_sprite].x = -2.0f; // TODO:  Animation
	unsigned team_sprite = g.push_quad_load(-1.0f, -0.422f, 0.9f, 0.9f, string("../resources/images/") + mc.team[mc.selected].number + string("-back.png"));
	g.push_arrow_box_left(-0.1f, -0.4f, 1.0f, 0.3f);
	g.push_arrow_box_right(-0.9f, 0.6f, 1.0f, 0.2f);
	unsigned team_hp_sprite = g.push_hp_bar(0.1f, -0.2f, get_hp_percent(mc.team[mc.selected]));
	unsigned exp_bar_index = g.push_quad_load(0.0f, -0.35f, 0.0f, 0.025f, string("../resources/images/exp.bmp"));
	unsigned hud_index = g.draw_list.size();
	mc.enemy_team[mc.enemy_selected].hud_index = hud_index;
	mc.enemy_team[mc.enemy_selected].last_hit_physical = 0;
	mc.enemy_team[mc.enemy_selected].last_hit_special = 0;
	mc.enemy_team[mc.enemy_selected].last_damage = 0;
	mc.enemy_team[mc.enemy_selected].sprite_index = enemy_sprite;
	mc.enemy_team[mc.enemy_selected].damage_dealt.clear();
	for (i = 0; i < 6; ++i) {
		mc.team[i].hp_bar_index = team_hp_sprite;
		mc.team[i].hud_index = hud_index;
		mc.team[i].exp_bar_index = 0;
		mc.team[i].last_hit_special = 0;
		mc.team[i].last_hit_physical = 0;
		mc.team[i].damage_dealt.clear();
		mc.team[i].sprite_index = team_sprite;
	}
	mc.team[mc.selected].exp_bar_index = exp_bar_index;
	resize_exp_bar(mc.team[mc.selected]);
	rebuild_battle_hud(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected]);
	mc.enemy_team[mc.enemy_selected].fought[mc.selected] = true;
	while (true) {
		old_team_selected = mc.selected;
		if (in_status(mc.team[mc.selected], string("RAGE")) && mc.team[mc.selected].queue.size() == 0)
			mc.team[mc.selected].queue.push_back(string("RAGE2"));
		if (in_status(mc.enemy_team[mc.enemy_selected], string("RAGE")) && mc.enemy_team[mc.enemy_selected].queue.size() == 0)
			mc.enemy_team[mc.enemy_selected].queue.push_back(string("RAGE2"));
		if (mc.team[mc.selected].queue.size() == 0) {
			bool any_valid_moves;
			any_valid_moves = false;
			for (unsigned i = 0; i < 4; ++i) {
				if (is_valid_move(mc.team[mc.selected], i)) {
					any_valid_moves = true;
				}
			}
			choices = any_valid_moves ? do_menu(string("COMBAT_SELECT")) : do_menu(string("COMBAT_SELECT_STRUGGLE"));
			while (choices.size() == 0 || choices[0] == -1) {
				choices = do_menu(string("COMBAT_SELECT"));
			}
			choices = remove_cancels(choices);
			if (choices[0] == 0) { // Player has selected FIGHT
				escape_attempts = 0;
				if (any_valid_moves) {
					if (in_status(mc.team[mc.selected], string("DISABLE")) && choices[1] == mc.team[mc.selected].disabled_move) {
						do_alert(string("That move is disabled!"));
						continue;
					}
					if (mc.team[mc.selected].pp[choices[1]] <= 0) {
						do_alert(string("There's no PP left!"));
						continue;
					}
					mc.team[mc.selected].queue.push_back(mc.team[mc.selected].moves[choices[1]]);
				}
				else {
					mc.team[mc.selected].queue.push_back(string("STRUGGLE"));
				}
			}
			else if (choices[0] == 1) { // Player has selected Pokemon
				if (in_status(mc.team[mc.selected], string("TRAP"))) {
					do_alert("Couldn't get away!");
					continue;
				}
				mc.team[mc.selected].queue.clear();
				mc.team[mc.selected].last_move = "";
				do_alert(get_nickname(mc.team[mc.selected]) + string("! That's enough!"));
				clear_volatile(mc.team[mc.selected]);
				mc.team[mc.selected].exp_bar_index = 0;
				mc.team[mc.selected].damage_dealt.clear();
				mc.selected = choices[1];
				mc.team[mc.selected].exp_bar_index = exp_bar_index;
				rebuild_battle_hud(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected]);
				mc.enemy_team[mc.enemy_selected].fought[mc.selected] = true;
				do_alert(string("Go, ") + get_nickname(mc.team[mc.selected]) + string("!"));
				mc.team[mc.selected].queue.clear();
				mc.team[mc.selected].last_move = "";
				mc.team[mc.selected].queue.push_back(string(""));
				mc.team[mc.selected].turn_count = 1;
			}
			else if (choices[0] == 2) { // Player has selected ITEM
				int out = 0;
				string o;
				if (!use_item(string("COMBAT"), choices, o))
					continue;
				mc.team[mc.selected].queue.insert(mc.team[mc.selected].queue.begin(), string(""));
				if (o.find("CAPTURE") == 0) {
					do_alert(string("You can't do that!")); // TODO:  Animation and correct fail message
				}
			}
			else if (choices[0] == 3) { // Player has selected RUN
				do_alert("No! There's no running from a trainer battle!");
				continue;
			}
		}
		count = 0.0;
		bool ai_chooses_switch = false;
		int a = 0, b = 0, fitness = 0;
		int best_fitness = 0;
		int current_fitness = 0;
		int best_fitness_index = -1;
		int num_KO = 0;
		for (i = 0; i < 6; ++i) {
			if (mc.enemy_team[i].defined && is_KO(mc.enemy_team[i])) {
				num_KO++;
			}
		}
		if (mc.enemy_team[mc.enemy_selected].queue.size() == 0) {
			if (random(0.0, 1.0) <= t.skill) {
				double temp3 = 0.0;
				index = get_smart_move(mc.team[old_team_selected], mc.enemy_team[mc.enemy_selected], t, true, 0, a, b, false, temp3);
				for (i = 0; i < 6; ++i) {
					if (mc.enemy_team[i].defined && !is_KO(mc.enemy_team[i])) {
						int hp_offset = get_smart_damage(mc.team[old_team_selected], mc.enemy_team[i], mc.team[old_team_selected].moves[index], t);
						if (i == mc.enemy_selected) {
							hp_offset = 0;
						}
						get_smart_move(mc.enemy_team[i], mc.team[old_team_selected], t, false, hp_offset, a, b, false, temp3);
						fitness = a - b;
						if (i == mc.enemy_selected) {
							current_fitness = fitness;
						}
						else {
							fitness--;
						}
						if (best_fitness_index == -1 || fitness > best_fitness) {
							best_fitness = fitness;
							best_fitness_index = i;
						}
					}
				}
				if (!t.overswitch) {
					if (best_fitness > 0 && best_fitness > 3 * current_fitness && best_fitness > 2 && num_KO >= t.no_switch) {
						ai_chooses_switch = true;
					}
				}
				else {
					if (best_fitness > current_fitness && num_KO >= t.no_switch) {
						ai_chooses_switch = true;
					}
				}
			}
			if (ai_chooses_switch) {
				clear_queue(mc.enemy_team[mc.enemy_selected]);
				clear_volatile(mc.enemy_team[mc.enemy_selected]);
				string old_name = get_nickname(mc.enemy_team[mc.enemy_selected]);
				mc.enemy_team[best_fitness_index].sprite_index = enemy_sprite;
				mc.enemy_team[best_fitness_index].hp_bar_index = mc.enemy_team[mc.enemy_selected].hp_bar_index;
				mc.enemy_selected = best_fitness_index;
				do_alert(t.display_name + string(" withdrew ") + old_name + string(" and sent out ") + get_nickname(mc.enemy_team[mc.enemy_selected]) + string("!"));
				mc.seen[mc.enemy_team[mc.enemy_selected].number] = true;
				mc.enemy_team[mc.enemy_selected].fought[mc.selected] = true;
				rebuild_battle_hud(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected]);
				mc.enemy_team[mc.enemy_selected].queue.push_back(string(""));
				mc.enemy_team[mc.enemy_selected].turn_count = 1;
			}
			else {
				for (i = 0; i < 4; i++) {
					if (is_valid_move(mc.enemy_team[mc.enemy_selected], i))
						count = count + 1.0;
				}
				if (count == 0.0) {
					mc.enemy_team[mc.enemy_selected].queue.push_back(string("STRUGGLE"));
				}
				else {
					index = int(random(0.0, count));
					if (random(0.0, 1.0) <= t.skill) {
						double temp3;
						index = get_smart_move(mc.enemy_team[mc.enemy_selected], mc.team[old_team_selected], t, false, 0, a, b, false, temp3);
					}
					int choice = -1;
					for (i = 0; i <= index; ++i) {
						if (!is_valid_move(mc.enemy_team[mc.enemy_selected], i))
							choice++;
						choice++;
					}
					if (choice == -1) {
						mc.enemy_team[mc.enemy_selected].queue.push_back(string("STRUGGLE"));
					}
					else {
						mc.enemy_team[mc.enemy_selected].queue.push_back(mc.enemy_team[mc.enemy_selected].moves[choice]);
					}
				}
			}
		}
		do_turn(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected]);
		if (in_status(mc.enemy_team[mc.enemy_selected], string("FLEE"))) {
			if (!in_status(mc.enemy_team[mc.enemy_selected], string("TRAP"))) {
				do_alert(string("ENEMY FLEE STATUS")); // TODO:  Swap logic
				remove_status(mc.enemy_team[mc.enemy_selected], string("FLEE"), true);
			}
			else {
			}
		}
		if (in_status(mc.team[mc.selected], string("FLEE"))) {
			if (!in_status(mc.team[mc.selected], string("TRAP"))) {
				do_alert(string("PLAYER FLEE STATUS")); // TODO:  Swap logic
				remove_status(mc.team[mc.selected], string("FLEE"), true);
			}
			else {
				do_alert("Couldn't get away!");
			}
		}
		remove_status(mc.team[mc.selected], string("FLINCH"), true);
		remove_status(mc.enemy_team[mc.enemy_selected], string("FLINCH"), true);

		if (is_KO(mc.team[mc.selected])) {
			do_alert(get_nickname(mc.team[mc.selected]) + string(" fainted!"));
			team_clear_volatile();
			for (i = 0; i < 6; ++i) {
				if (!mc.team[i].defined)
					continue;
				if (!is_KO(mc.team[i])) {
					break;
				}
			}
			if (i == 6) {
				// TODO:  Handle defeat
				do_alert(string("{PLAYER_NAME} is out of useable POK{e-accent}MON!"));
				do_alert(string("{PLAYER_NAME} blacked out!"));
				if (t.lose_message != "") {
					g.draw_list[enemy_trainer_sprite].x = 0.1f;
					g.draw_list[enemy_sprite].x = 2.0f;
					do_alert(t.lose_message);
				}
				g.draw_list.erase(g.draw_list.begin() + clear_point, g.draw_list.end());
				team_clear_volatile();
				return false;
			}
			choices = do_menu(string("COMBAT_MON"));
			choices = remove_cancels(choices);
			while (choices.size() == 0 || mc.team[choices[0]].curr_hp <= 0) {
				if (choices.size() > 0)
					do_alert(string("There's no will to fight!"));
				choices = do_menu(string("COMBAT_MON"));
				choices = remove_cancels(choices);
			}
			mc.team[mc.selected].exp_bar_index = 0;
			mc.team[mc.selected].damage_dealt.clear();
			mc.selected = choices[0];
			mc.team[mc.selected].exp_bar_index = exp_bar_index;
			mc.team[mc.selected].sprite_index = team_sprite;
			if (!is_KO(mc.enemy_team[mc.enemy_selected]))
				mc.enemy_team[mc.enemy_selected].fought[mc.selected] = true;
			rebuild_battle_hud(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected]);
		}
		if (is_KO(mc.enemy_team[mc.enemy_selected])) {
			do_alert(string("Enemy ") + get_nickname(mc.enemy_team[mc.enemy_selected]) + string(" fainted!"));
			int count = 0;
			for (unsigned i = 0; i < 6; ++i) {
				if ((mc.enemy_team[mc.enemy_selected].fought[i] || in_inventory(string("EXP.ALL"))) && mc.team[i].defined && !is_KO(mc.team[i])) {
					count++;
				}
			}
			for (unsigned i = 0; i < 6; ++i) {
				if ((mc.enemy_team[mc.enemy_selected].fought[i] || in_inventory(string("EXP.ALL"))) && mc.team[i].defined && !is_KO(mc.team[i])) {
					gain_exp(mc.team[i], mc.enemy_team[mc.enemy_selected], count, in_inventory(string("EXP.ALL")) ? 0.85 : 1.0);
					rebuild_battle_hud(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected]);
				}
			}
			bool found = false;
			bool smart_switch = false;
			best_fitness = 0;
			best_fitness_index = -1;
			if (num_KO + 1 >= t.no_switch && random(0.0, 1.0) <= t.skill) {
				smart_switch = true;
			}
			for (unsigned i = 0; i < 6; ++i) {
				if (smart_switch) {
					if (mc.enemy_team[i].defined && !is_KO(mc.enemy_team[i])) {
						double temp3;
						get_smart_move(mc.enemy_team[i], mc.team[mc.selected], t, false, 0, a, b, false, temp3);
						fitness = a - b;
						if (best_fitness_index == -1 || fitness > best_fitness) {
							found = true;
							best_fitness = fitness;
							best_fitness_index = i;
						}
					}
				}
				else {
					if (mc.enemy_team[i].defined && !is_KO(mc.enemy_team[i])) {
						best_fitness_index = i;
						found = true;
						break;
					}
				}
			}
			if (found) {
				mc.enemy_team[best_fitness_index].sprite_index = enemy_sprite;
				mc.enemy_team[best_fitness_index].hp_bar_index = mc.enemy_team[mc.enemy_selected].hp_bar_index;
				mc.enemy_selected = best_fitness_index; // TODO: Smart team selection
				do_alert(t.display_name + string(" sent out ") + get_nickname(mc.enemy_team[mc.enemy_selected]) + string("!"));
				mc.seen[mc.enemy_team[mc.enemy_selected].number] = true;
				mc.enemy_team[mc.enemy_selected].fought[mc.selected] = true;
				rebuild_battle_hud(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected]);
				mc.enemy_team[mc.enemy_selected].turn_count = 1;
			}
			if (!found)
				break;
		}
	}
	team_clear_volatile();
	g.draw_list[enemy_trainer_sprite].x = 0.1f;
	g.draw_list[enemy_sprite].x = 2.0f;
	do_alert(string("{PLAYER_NAME} defeated ") + t.display_name + string("!"));
	if (t.win_message != "")
		do_alert(t.win_message);
	int level = 0;
	for (int i = 0; i < 6; ++i) {
		if (mc.enemy_team[i].defined) {
			level = max(level, mc.enemy_team[i].level);
		}
	}
	mc.extra_winnings += level * t.payout;
	if (mc.extra_winnings > 0) {
		mc.money += mc.extra_winnings;
		do_alert(string("{PLAYER_NAME} got $") + to_string(mc.extra_winnings) + string(" for winning!"));
	}
	g.draw_list.erase(g.draw_list.begin() + clear_point, g.draw_list.end());
	return true;
}

bool engine::battle() { // wild pokemon
	int i;
	int index;
	int escape_attempts = 0;
	vector<int> choices;
	double count;
	int clear_point = g.draw_list.size();
	g.push_box(-1.1f, -1.1f, 2.2f, 2.2f);
	g.push_box(-1.0f, -1.0f, 2.0f, 0.6f);
	mc.selected = -1;
	mc.extra_winnings = 0;
	for (i = 0; i < 6; ++i) {
		if (mc.team[i].defined) {
			if (!is_KO(mc.team[i])) {
				mc.selected = i;
				break;
			}
		}
	}
	mc.team[mc.selected].queue.clear();
	mc.team[mc.selected].last_move = "";
	mc.enemy_team[mc.enemy_selected].queue.clear();
	mc.enemy_team[mc.enemy_selected].last_move = "";
	unsigned enemy_sprite = g.push_quad_load(0.1f, 0.1f, 0.9f, 0.9f, string("../resources/images/") + mc.enemy_team[mc.enemy_selected].number + string(".png"));
	mc.enemy_team[mc.enemy_selected].hp_bar_index = g.push_hp_bar(-0.7f, 0.7f, get_hp_percent(mc.enemy_team[mc.enemy_selected]));
	mc.enemy_team[mc.enemy_selected].hud_index = 0;
	unsigned player_sprite = g.push_quad_load(-1.0f, -0.422f, 0.9f, 0.9f, string("../resources/images/player-back.png"));
	do_alert(string("Wild ") + get_nickname(mc.enemy_team[mc.enemy_selected]) + string(" appeared!"));
	mc.enemy_team[mc.enemy_selected].turn_count = 0;
	mc.seen[mc.enemy_team[mc.enemy_selected].number] = true;
	do_alert(string("Go! ") + get_nickname(mc.team[mc.selected]) + string("!"));
	mc.team[mc.selected].turn_count = 1;
	g.draw_list[player_sprite].x = -2.0f; // TODO:  Animation
	unsigned team_sprite = g.push_quad_load(-1.0f, -0.422f, 0.9f, 0.9f, string("../resources/images/") + mc.team[mc.selected].number + string("-back.png"));
	g.push_arrow_box_left(-0.1f, -0.4f, 1.0f, 0.3f);
	g.push_arrow_box_right(-0.9f, 0.6f, 1.0f, 0.2f);
	unsigned team_hp_sprite = g.push_hp_bar(0.1f, -0.2f, get_hp_percent(mc.team[mc.selected]));
	unsigned exp_bar_index = g.push_quad_load(0.0f, -0.35f, 0.0f, 0.025f, string("../resources/images/exp.bmp"));
	unsigned hud_index = g.draw_list.size();
	mc.enemy_team[mc.enemy_selected].hud_index = hud_index;
	mc.enemy_team[mc.enemy_selected].last_hit_physical = 0;
	mc.enemy_team[mc.enemy_selected].last_hit_special = 0;
	mc.enemy_team[mc.enemy_selected].last_damage = 0;
	mc.enemy_team[mc.enemy_selected].sprite_index = enemy_sprite;
	mc.enemy_team[mc.enemy_selected].damage_dealt.clear();
	for (i = 0; i < 6; ++i) {
		mc.team[i].hp_bar_index = team_hp_sprite;
		mc.team[i].hud_index = hud_index;
		mc.team[i].exp_bar_index = 0;
		mc.team[i].last_hit_special = 0;
		mc.team[i].last_hit_physical = 0;
		mc.team[i].damage_dealt.clear();
		mc.team[i].sprite_index = team_sprite;
	}
	mc.team[mc.selected].exp_bar_index = exp_bar_index;
	resize_exp_bar(mc.team[mc.selected]);
	rebuild_battle_hud(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected]);
	mc.enemy_team[mc.enemy_selected].fought[mc.selected] = true;
	while (true) {
		if (in_status(mc.team[mc.selected], string("RAGE")) && mc.team[mc.selected].queue.size() == 0)
			mc.team[mc.selected].queue.push_back(string("RAGE2"));
		if (in_status(mc.enemy_team[mc.enemy_selected], string("RAGE")) && mc.enemy_team[mc.enemy_selected].queue.size() == 0)
			mc.enemy_team[mc.enemy_selected].queue.push_back(string("RAGE2"));
		if (mc.team[mc.selected].queue.size() == 0) {
			bool any_valid_moves;
			any_valid_moves = false;
			for (unsigned i = 0; i < 4; ++i) {
				if (is_valid_move(mc.team[mc.selected], i)) {
					any_valid_moves = true;
				}
			}
			choices = any_valid_moves ? do_menu(string("COMBAT_SELECT")) : do_menu(string("COMBAT_SELECT_STRUGGLE"));
			while (choices.size() == 0 || choices[0] == -1) {
				choices = do_menu(string("COMBAT_SELECT"));
			}
			choices = remove_cancels(choices);
			if (choices[0] == 0) { // Player has selected FIGHT
				escape_attempts = 0;
				if (any_valid_moves) {
					if (in_status(mc.team[mc.selected], string("DISABLE")) && choices[1] == mc.team[mc.selected].disabled_move) {
						do_alert(string("That move is disabled!"));
						continue;
					}
					if (mc.team[mc.selected].pp[choices[1]] <= 0) {
						do_alert(string("There's no PP left!"));
						continue;
					}
					mc.team[mc.selected].queue.push_back(mc.team[mc.selected].moves[choices[1]]);
				}
				else {
					mc.team[mc.selected].queue.push_back(string("STRUGGLE"));
				}
			}
			else if (choices[0] == 1) { // Player has selected Pokemon
				if (in_status(mc.team[mc.selected], string("TRAP"))) {
					do_alert("Couldn't get away!");
					continue;
				}
				mc.team[mc.selected].queue.clear();
				mc.team[mc.selected].last_move = "";
				do_alert(get_nickname(mc.team[mc.selected]) + string("! That's enough!"));
				clear_volatile(mc.team[mc.selected]);
				mc.team[mc.selected].exp_bar_index = 0;
				mc.team[mc.selected].damage_dealt.clear();
				mc.selected = choices[1];
				mc.team[mc.selected].exp_bar_index = exp_bar_index;
				rebuild_battle_hud(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected]);
				mc.enemy_team[mc.enemy_selected].fought[mc.selected] = true;
				do_alert(string("Go, ") + get_nickname(mc.team[mc.selected]) + string("!"));
				mc.team[mc.selected].queue.clear();
				mc.team[mc.selected].last_move = "";
				mc.team[mc.selected].queue.push_back(string(""));
				mc.team[mc.selected].turn_count = 1;
			}
			else if (choices[0] == 2) { // Player has selected ITEM
				int out = 0;
				string o;
				if (!use_item(string("COMBAT"), choices, o))
					continue;
				mc.team[mc.selected].queue.insert(mc.team[mc.selected].queue.begin(), string(""));
				if (o.find("CAPTURE") == 0) {
					o.erase(0, o.find(":") + 1);
					out = attempt_capture(stof(o), mc.enemy_team[mc.enemy_selected]);
				}
				switch (out) {
				case 0:
					do_alert("Oh, no! The POK{e-accent}MON broke free!");
					break;
				case 1:
					do_alert("Aww! It appeared to be caught!");
					break;
				case 2:
					do_alert("Aargh! Almost had it!");
					break;
				case 3:
					do_alert("Shoot! It was so close!");
					break;
				default:
					break;
				}
				// TODO:  Capture messages
				if (out == 4) {
					clear_volatile(mc.enemy_team[mc.enemy_selected]);
					mc.enemy_team[mc.enemy_selected].wild = false;
					mc.enemy_team[mc.enemy_selected].enemy = false;
					// TODO:  Nickname menu
					int count = 0;
					for (unsigned i = 0; i < 6; ++i) {
						if (mc.enemy_team[mc.enemy_selected].fought[i] && mc.team[i].defined && !is_KO(mc.team[i])) {
							count++;
						}
					}
					bool found = false;
					for (int i = 0; i < 6; ++i) {
						if (!mc.team[i].defined) {
							do_alert(string("Wild ") + get_nickname(mc.enemy_team[mc.enemy_selected]) + string(" was captured!"));
							mc.team[i] = mc.enemy_team[mc.enemy_selected];
							mc.team[i].wild = false;
							mc.team[i].enemy = false;
							found = true;
							break;
						}
					}
					if (!found) {
						for (int i = 0; i < STORAGE_MAX && !found; ++i) {
							for (int j = 0; j < STORAGE_MAX && !found; ++j) {
								if (!mc.storage[i][j].defined) {
									found = true;
									do_alert(string("Wild ") + get_nickname(mc.enemy_team[mc.enemy_selected]) + string(" was captured!"));
									mc.storage[i][j] = mc.enemy_team[mc.enemy_selected];
									mc.storage[i][j].wild = false;
									mc.storage[i][j].enemy = false;
									do_alert(get_nickname(mc.storage[i][j]) + string(" was sent to box ") + to_string(i + 1) + string("."));
								}
							}
						}
					}
					if (!found) {
						do_alert(string("There's no room in storage!"));
						do_alert(string("You have no choice but to let it go."));
					}
					mc.caught[mc.enemy_team[mc.enemy_selected].number] = true;
					count = 0;
					for (unsigned i = 0; i < 6; ++i) {
						if ((mc.enemy_team[mc.enemy_selected].fought[i] || in_inventory(string("EXP.ALL"))) && mc.team[i].defined && !is_KO(mc.team[i])) {
							count++;
						}
					}
					for (unsigned i = 0; i < 6; ++i) {
						if ((mc.enemy_team[mc.enemy_selected].fought[i] || in_inventory(string("EXP.ALL"))) && mc.team[i].defined && !is_KO(mc.team[i])) {
							gain_exp(mc.team[i], mc.enemy_team[mc.enemy_selected], count, in_inventory(string("EXP.ALL")) ? 0.85 : 1.0);
							rebuild_battle_hud(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected]);
						}
					}
					g.draw_list.erase(g.draw_list.begin() + clear_point, g.draw_list.end());
					return true;
					// TODO:  Implement storage
				}
			}
			else if (choices[0] == 3) { // Player has selected RUN
				escape_attempts++;
				mc.team[mc.selected].queue.clear();
				mc.team[mc.selected].last_move = "";
				if (!in_status(mc.team[mc.selected], string("TRAP")) && run_away(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected], escape_attempts)) {
					do_alert(string("Got away safely!"));
					g.draw_list.erase(g.draw_list.begin() + clear_point, g.draw_list.end());
					team_clear_volatile();
					return false;
				}
				else {
					do_alert(string("Couldn't get away!"));
				}
				mc.team[mc.selected].queue.push_back(string(""));
			}
		}
		count = 0.0;
		for (i = 0; i < 4; i++) {
			if (is_valid_move(mc.enemy_team[mc.enemy_selected], i))
				count = count + 1.0;
		}
		if (count == 0.0) {
			mc.enemy_team[mc.enemy_selected].queue.push_back(string("STRUGGLE"));
		}
		else {
			index = int(random(0.0, count));
			int choice = -1;
			for (i = 0; i <= index; ++i) {
				if (!is_valid_move(mc.enemy_team[mc.enemy_selected], i))
					choice++;
				choice++;
			}
			if (choice == -1) {
				mc.enemy_team[mc.enemy_selected].queue.push_back(string("STRUGGLE"));
			}
			else {
				mc.enemy_team[mc.enemy_selected].queue.push_back(mc.enemy_team[mc.enemy_selected].moves[choice]);
			}
		}
		do_turn(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected]);
		if (in_status(mc.enemy_team[mc.enemy_selected], string("FLEE"))) {
			if (!in_status(mc.enemy_team[mc.enemy_selected], string("TRAP"))) {
				do_alert(get_nickname(mc.enemy_team[mc.enemy_selected]) + string(" got away!"));
				break;
			}
			else {
				do_alert(get_nickname(mc.enemy_team[mc.enemy_selected]) + string(" couldn't get away!"));
			}
		}
		if (in_status(mc.team[mc.selected], string("FLEE"))) {
			if (!in_status(mc.team[mc.selected], string("TRAP"))) {
				do_alert("Got away safely!");
				break;
			}
			else {
				do_alert("Couldn't get away!");
			}
		}
		remove_status(mc.team[mc.selected], string("FLINCH"), true);
		remove_status(mc.enemy_team[mc.enemy_selected], string("FLINCH"), true);
		if (is_KO(mc.team[mc.selected])) {
			do_alert(get_nickname(mc.team[mc.selected]) + string(" fainted!"));
			team_clear_volatile();
			for (i = 0; i < 6; ++i) {
				if (!mc.team[i].defined)
					continue;
				if (!is_KO(mc.team[i])) {
					break;
				}
			}
			if (i == 6) {
				// TODO:  Handle defeat
				do_alert(string("{PLAYER_NAME} is out of useable POK{e-accent}MON!"));
				do_alert(string("{PLAYER_NAME} blacked out!"));
				g.draw_list.erase(g.draw_list.begin() + clear_point, g.draw_list.end());
				team_clear_volatile();
				return false;
			}
			choices = do_menu(string("ALERT_YES_NO"), string("Use next POK{e-accent}MON?"));
			if (choices[choices.size() - 1] == 1) {
				if (run_away(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected], escape_attempts)) {
					do_alert(string("Got away safely!"));
					g.draw_list.erase(g.draw_list.begin() + clear_point, g.draw_list.end());
					team_clear_volatile();
					return false;
				}
				else {
					do_alert(string("Couldn't get away!"));
				}
			}
			choices = do_menu(string("COMBAT_MON"));
			choices = remove_cancels(choices);
			while (choices.size() == 0 || mc.team[choices[0]].curr_hp <= 0) {
				if (choices.size() > 0)
					do_alert(string("There's no will to fight!"));
				choices = do_menu(string("COMBAT_MON"));
				choices = remove_cancels(choices);
			}
			mc.team[mc.selected].exp_bar_index = 0;
			mc.team[mc.selected].damage_dealt.clear();
			mc.selected = choices[0];
			mc.team[mc.selected].exp_bar_index = exp_bar_index;
			if (!is_KO(mc.enemy_team[mc.enemy_selected]))
				mc.enemy_team[mc.enemy_selected].fought[mc.selected] = true;
			rebuild_battle_hud(mc.team[mc.selected], mc.enemy_team[mc.enemy_selected]);
			mc.team[mc.selected].turn_count = 1;
		}
		if (is_KO(mc.enemy_team[mc.enemy_selected])) {
			do_alert(string("Enemy ") + get_nickname(mc.enemy_team[mc.enemy_selected]) + string(" fainted!"));
			int count = 0;
			for (unsigned i = 0; i < 6; ++i) {
				if ((mc.enemy_team[mc.enemy_selected].fought[i] || in_inventory(string("EXP.ALL"))) && mc.team[i].defined && !is_KO(mc.team[i])) {
					count++;
				}
			}
			for (unsigned i = 0; i < 6; ++i) {
				if ((mc.enemy_team[mc.enemy_selected].fought[i] || in_inventory(string("EXP.ALL"))) && mc.team[i].defined && !is_KO(mc.team[i])) {
					gain_exp(mc.team[i], mc.enemy_team[mc.enemy_selected], count, in_inventory(string("EXP.ALL")) ? 0.85 : 1.0);
				}
			}
			break;
		}
	}
	team_clear_volatile();
	if (mc.extra_winnings > 0) {
		mc.money += mc.extra_winnings;
		do_alert(string("{PLAYER_NAME} got $") + to_string(mc.extra_winnings) + string(" for winning!"));
	}
	g.draw_list.erase(g.draw_list.begin() + clear_point, g.draw_list.end());
	return true;
}

void engine::do_turn(mon& m1, mon& m2) {
	bool m1first = in_special(m1.queue[0], string("FIRST"));
	bool m2first = in_special(m2.queue[0], string("FIRST"));
	if (m1first && !m2first) {
		do_turn_inner(m1, m2);
	}
	else if (!m1first && m2first) {
		do_turn_inner(m2, m1);
	}
	else if (m1first && m2first && (get_stat(m1, SPEED) > get_stat(m2, SPEED))) {
		do_turn_inner(m1, m2);
	}
	else if (m1first && m2first && (get_stat(m1, SPEED) <= get_stat(m2, SPEED))) {
		do_turn_inner(m2, m1);
	}
	else if (get_stat(m1, SPEED) > get_stat(m2, SPEED)) {
		do_turn_inner(m1, m2);
	}
	else {
		do_turn_inner(m2, m1);
	}
}

int engine::damage(mon& attacker, mon& defender, string move, bool& crit, double &mul, bool &non_zero, bool force_no_crit, bool force_crit, bool skip_damage_range) {
	non_zero = false;
	double pow = stoi(moves[move].pow);
	mul = 1.0;
	crit = false;
	int buff = 0;
	for (unsigned i = 0; i < attacker.status.size(); ++i) {
		if (attacker.status[i] == this->get_buff_name(SPEED)) {
			buff++;
		}
		if (attacker.status[i] == this->get_debuff_name(SPEED)) {
			buff--;
		}
	}
	double chance = get_stat_modifier(buff) * all_mon[attacker.number].stats[SPEED] / 4.0;
	if (in_status(attacker, string("FOCUS")))
		chance *= 4.0;
	chance *= moves[move].crit_chance;
	if (random(0.0, 256.0) < chance) {
		crit = true;
	}
	if (force_crit)
		crit = true;
	if (force_no_crit)
		crit = false;
	double damage = 2.0 * attacker.level;
	damage = damage + 10.0;
	damage *= get_stat(attacker, moves[move].attack, false, crit);
	damage *= pow;
	damage /= 250.0;
	damage /= get_stat(defender, moves[move].defense, crit, false);
	damage += 2.0;
	if (moves[move].defense == SPECIAL && in_status(defender, string("LIGHTSCREEN")))
		damage /= 2.0;
	if (moves[move].defense == DEFENSE && in_status(defender, string("REFLECT")))
		damage /= 2.0;
	if ((get_type_1(attacker) == moves[move].type) || (get_type_2(attacker) == moves[move].type))
		damage *= 1.5;
	if (crit) {
		damage *= 1.5;
	}
	if (!skip_damage_range)
		damage *= random(0.85, 1.0);
	if (pow == 0.0)
		damage = 0.0;
	for (unsigned j = 0; j < moves[move].target.size(); ++j) {
		// Possible TODO: Remove multiplier from self-level damage moves.
		if (moves[move].target[j] == "SELF_LEVEL") {
			damage = int(double(attacker.level) * mul);
			if (crit)
				damage = int(double(damage)*1.5);
		}
		else if (moves[move].target[j].find("EXACT_DAMAGE") == 0) {
			string temp = moves[move].target[j];
			temp.erase(0, temp.find(":") + 1);
			damage = int(double(stoi(temp)) * mul);
			if (crit)
				damage = int(double(damage)*1.5);
		}
		else if (moves[move].target[j].find("HALF_HP") == 0) {
			damage = defender.curr_hp / 2;
			crit = false;
			mul = 1.0;
		}
		else if (moves[move].target[j].find("TWICE_SELF_DAMAGE_OVER_TURNS") == 0) {
			string temp = moves[move].target[j];
			temp.erase(0, temp.find(":") + 1);
			int turns = stoi(temp);
			damage = 0;
			for (int i = int(attacker.damage_dealt.size()) - 1; (i >= (int(attacker.damage_dealt.size()) - 3)) && (i >= 0); --i) {
				damage = damage + attacker.damage_dealt[i];
			}
			damage = damage * 2;
			crit = false;
			mul = 1.0;
		}
	}
	if (damage > 0.0) {
		non_zero = true;
	}
	if (get_type_1(defender) != "") {
		mul *= types[moves[move].type][get_type_1(defender)];
		damage *= types[moves[move].type][get_type_1(defender)];
	}
	if (get_type_2(defender) != "") {
		mul *= types[moves[move].type][get_type_2(defender)];
		damage *= types[moves[move].type][get_type_2(defender)];
	}
	return int(damage);
}

string engine::get_buff_name(STAT s) {
	switch (s) {
	case ATTACK:
		return string("ATTACK_UP");
	case DEFENSE:
		return string("DEFENSE_UP");
	case SPEED:
		return string("SPEED_UP");
	case SPECIAL:
		return string("SPECIAL_UP");
	default:
		return string("");
	}
}

string engine::get_debuff_name(STAT s) {
	switch (s) {
	case ATTACK:
		return string("ATTACK_DOWN");
	case DEFENSE:
		return string("DEFENSE_DOWN");
	case SPEED:
		return string("SPEED_DOWN");
	case SPECIAL:
		return string("SPECIAL_DOWN");
	default:
		return string("");
	}
}

double engine::get_accuracy_modifier(mon& m) {
	int buff = 0;
	for (unsigned i = 0; i < m.status.size(); ++i) {
		if (m.status[i] == "ACCURACY_UP")
			buff++;
		if (m.status[i] == "ACCURACY_DOWN")
			buff--;
	}
	if (buff < -6)
		buff = -6;
	if (buff > 6)
		buff = 6;
	switch (buff) {
	case -6:
		return 3.0 / 9.0;
	case -5:
		return 3.0 / 8.0;
	case -4:
		return 3.0 / 7.0;
	case -3:
		return 3.0 / 6.0;
	case -2:
		return 3.0 / 5.0;
	case -1:
		return 3.0 / 4.0;
	case 0:
		return 3.0 / 3.0;
	case 1:
		return 4.0 / 3.0;
	case 2:
		return 5.0 / 3.0;
	case 3:
		return 6.0 / 3.0;
	case 4:
		return 7.0 / 3.0;
	case 5:
		return 8.0 / 3.0;
	case 6:
		return 9.0 / 3.0;
	default:
		return 1.0;
	}
}

double engine::get_evasion_modifier(mon& m) {
	int buff = 0;
	for (unsigned i = 0; i < m.status.size(); ++i) {
		if (m.status[i] == "EVASION_UP")
			buff++;
		if (m.status[i] == "EVASION_DOWN")
			buff--;
	}
	if (buff < -6)
		buff = -6;
	if (buff > 6)
		buff = 6;
	switch (buff) {
	case -6:
		return 3.0 / 9.0;
	case -5:
		return 3.0 / 8.0;
	case -4:
		return 3.0 / 7.0;
	case -3:
		return 3.0 / 6.0;
	case -2:
		return 3.0 / 5.0;
	case -1:
		return 3.0 / 4.0;
	case 0:
		return 3.0 / 3.0;
	case 1:
		return 4.0 / 3.0;
	case 2:
		return 5.0 / 3.0;
	case 3:
		return 6.0 / 3.0;
	case 4:
		return 7.0 / 3.0;
	case 5:
		return 8.0 / 3.0;
	case 6:
		return 9.0 / 3.0;
	default:
		return 1.0;
	}
}

double engine::get_stat_modifier(int buff) {
	if (buff < -6)
		buff = -6;
	if (buff > 6)
		buff = 6;
	switch (buff) {
	case -6:
		return 2.0 / 8.0;
	case -5:
		return 2.0 / 7.0;
	case -4:
		return 2.0 / 6.0;
	case -3:
		return 2.0 / 5.0;
	case -2:
		return 2.0 / 4.0;
	case -1:
		return 2.0 / 3.0;
	case 0:
		return 2.0 / 2.0;
	case 1:
		return 3.0 / 2.0;
	case 2:
		return 4.0 / 2.0;
	case 3:
		return 5.0 / 2.0;
	case 4:
		return 6.0 / 2.0;
	case 5:
		return 7.0 / 2.0;
	case 6:
		return 8.0 / 2.0;
	default:
		return 1.0;
	}
}

string engine::get_type_1(mon& m) {
	for (unsigned i = 0; i < m.status.size(); ++i) {
		if (m.status[i].find("TYPE1:") == 0) {
			string s = m.status[i];
			s.erase(0, s.find(":") + 1);
			return s;
		}
	}
	return all_mon[m.number].type_1;
}

string engine::get_type_2(mon& m) {
	for (unsigned i = 0; i < m.status.size(); ++i) {
		if (m.status[i].find("TYPE2:") == 0) {
			string s = m.status[i];
			s.erase(0, s.find(":") + 1);
			return s;
		}
	}
	return all_mon[m.number].type_2;
}

int engine::get_stat(mon& m, STAT s, bool ignore_buffs, bool ignore_debuffs) {
	int ret;
	if (s == HP) {
		ret = all_mon[m.number].stats[s] + m.IV[s];
		ret = ret * 2;
		ret = ret + (sqrt(min(65535, m.EV[s])) / 4);
		ret = ret * m.level;
		ret = ret / 100;
		ret = ret + m.level + 10;
	}
	else {
		ret = all_mon[m.number].stats[s] + m.IV[s];
		ret = ret * 2;
		ret = ret + (sqrt(min(65535, m.EV[s])) / 4);
		ret = ret * m.level;
		ret = ret / 100;
		ret = ret + 5;
		if (!ignore_debuffs) {
			if (s == ATTACK && in_status(m, string("BURN")))
				ret = ret / 2;
			if (s == SPEED && in_status(m, string("PARALYZE")))
				ret = ret / 4;
		}
	}
	int buff = 0;
	for (unsigned i = 0; i < m.status.size(); ++i) {
		if (!ignore_buffs) {
			if (m.status[i] == this->get_buff_name(s)) {
				buff++;
			}
		}
		if (!ignore_debuffs) {
			if (m.status[i] == this->get_debuff_name(s)) {
				buff--;
			}
		}
	}
	return int(ret * get_stat_modifier(buff));
}

void engine::init_items() {
	ifstream f("../resources/data/items.dat");
	string line;
	string key;
	string temp;
	while (f.is_open()) {
		while (std::getline(f, line)) {
			if (line.find("ITEM:") == 0) {
				line.erase(0, string("ITEM:").length());
				key = line;
				items[key].defined = true;
			}
			else if (line.find("USE:") == 0) {
				line.erase(0, string("USE:").length());
				temp = line;
				while (line.find("|") != -1) {
					line.erase(0, line.find("|") + 1);
					temp.erase(temp.find("|"), temp.length());
					items[key].use.insert(items[key].use.begin(), temp);
					temp = line;
				}
				items[key].use.insert(items[key].use.begin(), temp);
			}
			else if (line.find("EFFECT:") == 0) {
				line.erase(0, string("EFFECT:").length());
				temp = line;
				while (line.find("|") != -1) {
					line.erase(0, line.find("|") + 1);
					temp.erase(temp.find("|"), temp.length());
					items[key].effect.insert(items[key].effect.begin(), temp);
					temp = line;
				}
				items[key].effect.insert(items[key].effect.begin(), temp);
			}
			else if (line.find("PRICE:") == 0) {
				line.erase(0, string("PRICE:").length());
				items[key].price = stoi(line);
			}
		}
		f.close();
	}
}

void engine::init_levels() {
	ifstream f("../resources/data/levels.dat");
	string line;
	while (f.is_open()) {
		while (std::getline(f, line))
			init_level(line);
		f.close();
	}
}

void engine::init_level(string levelname) {
	ifstream f(("../resources/levels/" + levelname + ".dat").c_str());
	string line;
	std::vector<int> empty;
	unsigned count;
	while (f.is_open()) {
		std::getline(f, line);
		if (line == "NAME") {
			std::getline(f, line);
			levels[levelname].name = line;
			continue;
		}
		if (line == "NPCS") {
			std::getline(f, line);
			while (line != "TELEPORT" && line != "DATA" && line != "ENCOUNTERS" && line != "LEVELS" && line != "TRAINERS" && line.find("WATER_ENCOUNTERS_") != 0) {
				character c;
				string s;
				levels[levelname].characters.push_back(c);
				s = line;
				s.erase(s.find(" "), s.length());
				line.erase(0, line.find(" ") + 1);
				levels[levelname].characters[levels[levelname].characters.size() - 1].name = s;
				s = line;
				s.erase(s.find(" "), s.length());
				line.erase(0, line.find(" ") + 1);
				levels[levelname].characters[levels[levelname].characters.size() - 1].image = s;
				s = line;
				s.erase(s.find(" "), s.length());
				line.erase(0, line.find(" ") + 1);
				levels[levelname].characters[levels[levelname].characters.size() - 1].origin.x = stoi(s);
				s = line;
				if (s.find(" ") != -1)
					s.erase(s.find(" "), s.length());
				if (line.find(" ") != -1)
					line.erase(0, line.find(" ") + 1);
				levels[levelname].characters[levels[levelname].characters.size() - 1].origin.y = stoi(s);
				levels[levelname].characters[levels[levelname].characters.size() - 1].origin.level = levelname;
				levels[levelname].characters[levels[levelname].characters.size() - 1].loc = levels[levelname].characters[levels[levelname].characters.size() - 1].origin;
				s = line;
				if (s.find("INACTIVE") != -1) {
					mc.active[levels[levelname].characters[levels[levelname].characters.size() - 1].name] = false;
				}
				else {
					mc.active[levels[levelname].characters[levels[levelname].characters.size() - 1].name] = true;
				}
				if (s.find("INCORPOREAL") != -1) {
					levels[levelname].characters[levels[levelname].characters.size() - 1].incorporeal = true;
				}
				else {
					levels[levelname].characters[levels[levelname].characters.size() - 1].incorporeal = false;
				}
				if (s.find("FARSPEAK") != -1) {
					levels[levelname].characters[levels[levelname].characters.size() - 1].far = true;
				}
				else {
					levels[levelname].characters[levels[levelname].characters.size() - 1].far = false;
				}
				if (s.find("NO_OFFSET") != -1) {
					levels[levelname].characters[levels[levelname].characters.size() - 1].no_offset = true;
				}
				else {
					levels[levelname].characters[levels[levelname].characters.size() - 1].no_offset = false;
				}
				if (s.find("NOLOOK") != -1) {
					levels[levelname].characters[levels[levelname].characters.size() - 1].nolook = true;
				}
				else {
					levels[levelname].characters[levels[levelname].characters.size() - 1].nolook = false;
				}
				if (s.find("WANDER") != -1) {
					levels[levelname].characters[levels[levelname].characters.size() - 1].wander = true;
				}
				else {
					levels[levelname].characters[levels[levelname].characters.size() - 1].wander = false;
				}
				if (s.find("PUSHABLE") != -1) {
					levels[levelname].characters[levels[levelname].characters.size() - 1].pushable = true;
				}
				else {
					levels[levelname].characters[levels[levelname].characters.size() - 1].pushable = false;
				}
				if (s.find("TELEPORTABLE") != -1) {
					levels[levelname].characters[levels[levelname].characters.size() - 1].teleportable = true;
				}
				else {
					levels[levelname].characters[levels[levelname].characters.size() - 1].teleportable = false;
				}
				levels[levelname].characters[levels[levelname].characters.size() - 1].dir = DOWN;
				if (s.find("LEFT") != -1)
					levels[levelname].characters[levels[levelname].characters.size() - 1].dir = LEFT;
				if (s.find("RIGHT") != -1)
					levels[levelname].characters[levels[levelname].characters.size() - 1].dir = RIGHT;
				if (s.find("UP") != -1)
					levels[levelname].characters[levels[levelname].characters.size() - 1].dir = UP;
				if (s.find("DOWN") != -1)
					levels[levelname].characters[levels[levelname].characters.size() - 1].dir = DOWN;
				levels[levelname].characters[levels[levelname].characters.size() - 1].origin_dir = levels[levelname].characters[levels[levelname].characters.size() - 1].dir;
				mc.interaction[levels[levelname].characters[levels[levelname].characters.size() - 1].name] = 0;
				std::getline(f, line);
				if (line == "FORCE_INTERACTION") {
					std::getline(f, line);
					while (line != "END") {
						location l;
						s = line;
						s.erase(s.find(" "), s.size());
						line.erase(0, line.find(" ") + 1);
						l.x = double(stoi(s));
						l.y = double(stoi(line));
						levels[levelname].characters[levels[levelname].characters.size() - 1].force_interactions.push_back(l);
						std::getline(f, line);
					}
					std::getline(f, line);
				}
				if (line == "INTERACTIONS") {
					std::getline(f, line);
					while (line != "END") {
						levels[levelname].characters[levels[levelname].characters.size() - 1].interactions.push_back(line);
						std::getline(f, line);
					}
					std::getline(f, line);
				}
			}
		}
		if (line == "TRAINERS") {
			std::getline(f, line);
			trainer d;
			while (line != "DATA" && line != "ENCOUNTERS" && line != "LEVELS" && line != "TELEPORT" && line != "END" && line.find("WATER_ENCOUNTERS_") != 0) {
				string s, s2, s3;
				unsigned i = 1;
				unsigned count = 1;
				s = line;
				s2 = s;
				s.erase(0, s.find(" ") + 1);
				s2.erase(s2.find(" "), s2.length());
				d.name = s2;
				d.skip_accuracy_check = false;
				d.overswitch = false;
				d.no_switch = 0;
				if (s.find("{") == 0) {
					// Special modifiers here
					s2 = s;
					s.erase(0, s.find("}") + 2);
					s2.erase(s2.find("}") + 1, s2.length());
					if (s2.find("NO_ACCURACY_CHECK") != -1) {
						d.skip_accuracy_check = true;
					}
					if (s2.find("NO_SWITCH:1") != -1) {
						d.no_switch = 1;
					}
					if (s2.find("NO_SWITCH:1") != -1) {
						d.no_switch = 1;
					}
					if (s2.find("NO_SWITCH:2") != -1) {
						d.no_switch = 2;
					}
					if (s2.find("NO_SWITCH:3") != -1) {
						d.no_switch = 3;
					}
					if (s2.find("NO_SWITCH:4") != -1) {
						d.no_switch = 4;
					}
					if (s2.find("NO_SWITCH:5") != -1) {
						d.no_switch = 5;
					}
					if (s2.find("NO_SWITCH:6") != -1) {
						d.no_switch = 6;
					}
					if (s2.find("OVERSWITCH") != -1) {
						d.overswitch = true;
					}
				}
				s2 = s;
				s.erase(0, s.find(" ") + 1);
				s2.erase(s2.find(" "), s2.length());
				d.payout = stoi(s2);
				s2 = s;
				s.erase(0, s.find(" ") + 1);
				s2.erase(s2.find(" "), s2.length());
				d.skill = stof(s2);
				s2 = s;
				s.erase(0, s.find(" ") + 1);
				s2.erase(s2.find(" "), s2.length());
				d.knowledge = stof(s2);
				s2 = s;
				s.erase(0, s.find(" ") + 1);
				s2.erase(s2.find(" "), s2.length());
				d.display_name = s2;
				s2 = s;
				s.erase(0, s.find(" ") + 1);
				s2.erase(s2.find(" "), s2.length());
				d.image = s2;
				s2 = s;
				while (count > 0) {
					if (s2[i] == '{')
						count++;
					if (s2[i] == '}')
						count--;
					++i;
				}
				s2.erase(i - 1, s2.length());
				s2.erase(0, 1);
				d.win_message = s2;
				s.erase(0, i);
				s2 = s;
				i = 1;
				count = 1;
				while (count > 0) {
					if (s2[i] == '{')
						count++;
					if (s2[i] == '}')
						count--;
					++i;
				}
				s2.erase(i - 1, s2.length());
				s2.erase(0, 1);
				d.lose_message = s2;
				s.erase(0, i + 1);
				i = 0;
				for (unsigned j = 0; j < 6; ++j) {
					d.team[j].defined = false;
				}
				while (s != "") {
					if (s.find("|") == 0)
						s.erase(0, 1);
					s2 = s;
					s2.erase(s2.find(":"), s2.length());
					s.erase(0, s.find(":") + 1);
					s3 = s;
					if (s3.find("|") != -1) {
						s3.erase(s3.find("|"), s3.length());
					}
					if (s3.find(":") != -1) {
						s3.erase(s3.find(":"), s3.length());
					}
					count = stoi(s3);
					make_mon(s2, count, d.team[i]);
					d.team[i].wild = false;
					s.erase(0, s3.length());
					if (s.find(":") == 0) {
						s.erase(0, 1);
					}
					s3 = s;
					if (s.find("|") != -1) {
						s.erase(0, s.find("|") + 1);
						s3.erase(s3.find("|"), s3.length());
					}
					else {
						s = "";
					}
					if (s3 != "") {
						count = 0;
						while (count < 4) {
							s2 = s3;
							if (s2.find("|") != -1) {
								s2.erase(s2.find("|"), s2.length());
							}
							if (s2.find(",") == 0) {
								s2 = "";
							}
							if (s2.find(",") != -1) {
								s2.erase(s2.find(","), s2.length());
							}
							create_move(d.team[i], s2, count, true);
							if (!moves[s2].defined) {
								printf("\nERROR: Invalid move %s defined", s2.c_str());
							}
							if (s3.find(",") != -1) {
								s3.erase(0, s3.find(",") + 1);
							}
							count++;
						}
					}
					i = i + 1;
				}
				levels[levelname].trainers[d.name] = d;
				std::getline(f, line);
			}
			std::getline(f, line);
		}
		if (line == "TELEPORT")
		{
			std::getline(f, line);
			std::pair<location, location> temp;
			while (line != "NEIGHBORS" && line != "DATA" && line != "ENCOUNTERS" && line != "LEVELS" && line.find("WATER_ENCOUNTERS_") != 0) {
				temp.first.level = levelname;
				temp.first.x = double(stoi(line));
				line.erase(0, line.find(" ") + 1);
				temp.first.y = double(stoi(line));
				line.erase(0, line.find(" ") + 1);
				temp.second.level = "";
				while (line[0] != ' ') {
					temp.second.level += line[0];
					line.erase(0, 1);
				}
				line.erase(0, 1);
				temp.second.x = double(stoi(line));
				line.erase(0, line.find(" ") + 1);
				temp.second.y = double(stoi(line));
				temp.first.allow_npcs = true;
				temp.first.allow_player = true;
				if (line.find(" ") != -1) {
					temp.first.allow_npcs = false;
					temp.first.allow_player = false;
					if (line.find("PLAYER") != -1) {
						temp.first.allow_player = true;
					}
					if (line.find("NPC") != -1) {
						temp.first.allow_npcs = true;
					}
				}
				levels[levelname].teleport.push_back(temp);
				std::getline(f, line);
			}
		}
		if (line == "NEIGHBORS") {
			std::getline(f, line);
			while (line.find(" ") != -1) {
				neighbor n;
				n.level = line;
				n.level.erase(n.level.find(" "), n.level.size());
				line.erase(0, line.find(" ") + 1);
				n.x = stof(line);
				line.erase(0, line.find(" ") + 1);
				n.y = stof(line);
				levels[levelname].neighbors.push_back(n);
				std::getline(f, line);
			}
		}
		if (line == "ENCOUNTERS") {
			std::getline(f, line);
			while (line.size() > 0) {
				levels[levelname].encounters.push_back(stoi(line));
				if (line.find(' ') == -1)
					break;
				line.erase(0, line.find(' ') + 1);
				while (line.find(' ') == 0)
					line.erase(0, 1);
			}
			std::getline(f, line);
		}
		if (line == "MEGA_ENCOUNTERS") {
			std::getline(f, line);
			while (line.size() > 0) {
				levels[levelname].mega_encounters.push_back(stoi(line));
				if (line.find(' ') == -1)
					break;
				line.erase(0, line.find(' ') + 1);
				while (line.find(' ') == 0)
					line.erase(0, 1);
			}
			std::getline(f, line);
		}
		if (line == "WATER_ENCOUNTERS_OLD") {
			std::getline(f, line);
			while (line.size() > 0) {
				levels[levelname].water_encounters_old.push_back(stoi(line));
				if (line.find(' ') == -1)
					break;
				line.erase(0, line.find(' ') + 1);
				while (line.find(' ') == 0)
					line.erase(0, 1);
			}
			std::getline(f, line);
		}
		if (line == "WATER_ENCOUNTERS_GOOD") {
			std::getline(f, line);
			while (line.size() > 0) {
				levels[levelname].water_encounters_good.push_back(stoi(line));
				if (line.find(' ') == -1)
					break;
				line.erase(0, line.find(' ') + 1);
				while (line.find(' ') == 0)
					line.erase(0, 1);
			}
			std::getline(f, line);
		}
		if (line == "WATER_ENCOUNTERS_SUPER") {
			std::getline(f, line);
			while (line.size() > 0) {
				levels[levelname].water_encounters_super.push_back(stoi(line));
				if (line.find(' ') == -1)
					break;
				line.erase(0, line.find(' ') + 1);
				while (line.find(' ') == 0)
					line.erase(0, 1);
			}
			std::getline(f, line);
		}
		if (line == "LEVELS") {
			std::getline(f, line);
			levels[levelname].level_range.first = stoi(line);
			line.erase(0, line.find('-') + 1);
			levels[levelname].level_range.second = stoi(line);
			std::getline(f, line);
		}
		if (line == "MEGA_LEVELS") {
			std::getline(f, line);
			levels[levelname].mega_level_range.first = stoi(line);
			line.erase(0, line.find('-') + 1);
			levels[levelname].mega_level_range.second = stoi(line);
			std::getline(f, line);
		}
		if (line == "DATA") {
			while (std::getline(f, line)) {
				levels[levelname].data.push_back(empty);
				while (line.length() > 0) {
					levels[levelname].data[levels[levelname].data.size() - 1].push_back(stoi(line));
					count = 0;
					while (line[count] != '\n' && line[count] != ' ' && count < line.length())
						count++;
					while (line[count] == ' ')
						count++;
					line.erase(0, count);
				}
			}
		}
		f.close();
	}
}

void engine::init_moves() {
	string line, key;
	ifstream f("../resources/data/moves.dat");
	char a = f.get();
	while (a != EOF && f.is_open()) {
		line = "";
		while (a != ':') {
			line = line + a;
			a = f.get();
		}
		while (a == ':' || a == ' ')
			a = f.get();
		if (line == "MOVE") {
			line = "";
			while (a != '\r' && a != '\n') {
				line = line + a;
				a = f.get();
			}
			key = line;
			moves[key].defined = true;
			moves[key].name = key;
		}
		else if (line == "TYPE") {
			line = "";
			while (a != '\r' && a != '\n') {
				line = line + a;
				a = f.get();
			}
			moves[key].type = line;
		}
		else if (line == "POWER") {
			line = "";
			while (a != '\r' && a != '\n') {
				line = line + a;
				a = f.get();
			}
			moves[key].pow = line;
		}
		else if (line == "DEFENSE") {
			line = "";
			while (a != '\r' && a != '\n') {
				line = line + a;
				a = f.get();
			}
			if (line == "SPECIAL") {
				moves[key].attack = SPECIAL;
				moves[key].defense = SPECIAL;
			}
		}
		else if (line == "CRITICAL") {
			line = "";
			while (a != '\r' && a != '\n') {
				line = line + a;
				a = f.get();
			}
			if (line == "HIGH") {
				moves[key].crit_chance = 8.0f;
			}
			else if (line == "NORMAL") {
				moves[key].crit_chance = 1.0f;
			}
			else if (line == "NONE") {
				moves[key].crit_chance = 0.0f;
			}
		}
		else if (line == "QUEUE_ONLY") {
			line = "";
			while (a != '\r' && a != '\n') {
				line = line + a;
				a = f.get();
			}
			if (line == "YES") {
				moves[key].queue_only = true;
			}
		}
		else if (line == "ACC") {
			line = "";
			while (a != '\r' && a != '\n') {
				line = line + a;
				a = f.get();
			}
			moves[key].acc = stoi(line);
		}
		else if (line == "PP") {
			line = "";
			while (a != '\r' && a != '\n') {
				line = line + a;
				a = f.get();
			}
			moves[key].pp = stoi(line);
		}
		else if (line == "SELF") {
			while (a != '\r' && a != '\n' && a != EOF) {
				line = "";
				while (a != ' ' && a != '\r' && a != '\n' && a != EOF) {
					line = line + a;
					a = f.get();
				}
				while (a == ' ')
					a = f.get();
				moves[key].self.push_back(line);
			}
		}
		else if (line == "TARGET") {
			while (a != '\r' && a != '\n' && a != EOF) {
				line = "";
				while (a != ' ' && a != '\r' && a != '\n' && a != EOF) {
					line = line + a;
					a = f.get();
				}
				while (a == ' ')
					a = f.get();
				moves[key].target.push_back(line);
			}
		}
		else if (line == "QUEUE") {
			while (a != '\r' && a != '\n' && a != EOF) {
				line = "";
				while (a != ' ' && a != '\r' && a != '\n' && a != EOF) {
					line = line + a;
					a = f.get();
				}
				while (a == ' ')
					a = f.get();
				moves[key].queue.push_back(line);
			}
		}
		else if (line == "SPECIAL") {
			while (a != '\r' && a != '\n' && a != EOF) {
				line = "";
				while (a != ' ' && a != '\r' && a != '\n' && a != EOF) {
					line = line + a;
					a = f.get();
				}
				while (a == ' ')
					a = f.get();
				moves[key].special.push_back(line);
			}
		}
		else if (line == "ADDITIONAL") {
			while (a != '\r' && a != '\n' && a != EOF) {
				line = "";
				while (a != ' ' && a != '\r' && a != '\n' && a != EOF) {
					line = line + a;
					a = f.get();
				}
				while (a == ' ')
					a = f.get();
				moves[key].additional.push_back(line);
			}
		}
		else if (line == "DESC") {
			line = "";
			while (a != '\r' && a != '\n' && a != EOF) {
				line = line + a;
				a = f.get();
			}
			moves[key].desc = line;
		}
		else if (line == "NOTES") {
			line = "";
			while (a != '\r' && a != '\n' && a != EOF) {
				line = line + a;
				a = f.get();
			}
			moves[key].notes = line;
		}

		while (a != '\n' && a != EOF)
			a = f.get();
		while (a == '\n' && a != EOF)
			a = f.get();
		if (a == EOF) {
			f.close();
			break;
		}
	}
}

void engine::init_tm() {
	string line, index;
	ifstream f("../resources/data/TM.dat");
	while (std::getline(f, line)) {
		index = line;
		index.erase(index.find(":"), index.size());
		line.erase(0, line.find(":") + 1);
		TM[stoi(index)] = line;
	}
	f.close();
}

void engine::init_hm() {
	string line, index;
	ifstream f("../resources/data/HM.dat");
	while (std::getline(f, line)) {
		index = line;
		index.erase(index.find(":"), index.size());
		line.erase(0, line.find(":") + 1);
		HM[stoi(index)] = line;
	}
	f.close();
}

void engine::init_mon() {
	string line, key, temp;
	ifstream f("../resources/data/mon.dat");
	char a = 1;
	int level = 0;
	while (f.is_open() && !f.eof()) {
		line = "";
		std::getline(f, temp);
		if (temp.find(":") != -1) {
			line = temp;
			line.erase(line.find(":"), line.length());
			temp.erase(0, temp.find(":") + 1);
			while (temp.find(" ") == 0)
				temp.erase(0, 1);
		}
		if (line == "NUMBER") {
			key = temp;
			all_mon[key].number = temp;
			all_mon[key].defined = true;
		}
		else if (line == "NAME") {
			all_mon[key].name = temp;
		}
		else if (line == "SURNAME") {
			all_mon[key].name = all_mon[key].name + temp;
		}
		else if (line == "SWITCH") {
		}
		else if (line == "TYPE1") {
			all_mon[key].type_1 = temp;
		}
		else if (line == "TYPE2") {
			all_mon[key].type_2 = temp;
		}
		else if (line == "DEX") {
			all_mon[key].dex = temp;
		}
		else if (line == "DEX1") {
			all_mon[key].dex = temp;
		}
		else if (line == "DEX2") {
			all_mon[key].dex = all_mon[key].dex + string(" ") + temp;
		}
		else if (line == "EXP_YIELD") {
			all_mon[key].exp_yield = stoi(temp);
		}
		else if (line == "CATCHRATE") {
			all_mon[key].catchrate = stoi(temp);
		}
		else if (line == "HEIGHT") {
			all_mon[key].height = stoi(temp);
		}
		else if (line == "WEIGHT") {
			all_mon[key].weight = stof(temp);
		}
		else if (line == "HP") {
			all_mon[key].stats[HP] = stoi(temp);
		}
		else if (line == "ATTACK") {
			all_mon[key].stats[ATTACK] = stoi(temp);
		}
		else if (line == "DEFENSE") {
			all_mon[key].stats[DEFENSE] = stoi(temp);
		}
		else if (line == "SPECIAL") {
			all_mon[key].stats[SPECIAL] = stoi(temp);
		}
		else if (line == "SPEED") {
			all_mon[key].stats[SPEED] = stoi(temp);
		}
		else if (line == "MOVES") {
			pair<int, string> tempmove;
			std::getline(f, temp);
			while (temp.find(":") == -1) {
				line = temp;
				line.erase(line.find(" "), line.length());
				temp.erase(0, temp.find(" ") + 1);
				tempmove.first = stoi(line);
				tempmove.second = temp;
				all_mon[key].learned.push_back(tempmove);
				std::getline(f, temp);
			}
			line = temp;
			line.erase(line.find(":"), line.length());
			temp.erase(0, temp.find(":") + 1);
		}
		if (line == "EVOLUTION") {
			pair<string, string> tempev;
			std::getline(f, temp);
			while (temp.find("TM") != 0) {
				pair<string, string> tempev;
				line = temp;
				line.erase(line.find(":"), line.length());
				temp.erase(0, temp.find(":") + 1);
				tempev.first = line;
				tempev.second = temp;
				all_mon[key].evolution.push_back(tempev);
				std::getline(f, temp);
			}
			line = temp;
			line.erase(line.find(":"), line.length());
			temp.erase(0, temp.find(":") + 1);
		}
		if (line == "TM") {
			while (temp.find(" ") == 0)
				temp.erase(0, 1);
			while (temp != "") {
				line = temp;
				if (temp.find(" ") != -1) {
					line.erase(line.find(" "), line.length());
					temp.erase(0, temp.find(" ") + 1);
				}
				else
					temp = "";
				all_mon[key].TM[stoi(line)] = true;
			}
		}
		if (line == "HM") {
			while (temp.find(" ") == 0)
				temp.erase(0, 1);
			while (temp != "") {
				line = temp;
				if (temp.find(" ") != -1) {
					line.erase(line.find(" "), line.length());
					temp.erase(0, temp.find(" ") + 1);
				}
				else
					temp = "";
				all_mon[key].HM[stoi(line)] = true;
			}
		}
	}
}

void engine::init_status() {
	string line, key;
	ifstream f("../resources/data/status.dat");
	char a = f.get();
	while (f.is_open()) {
		line = "";
		while (a == '\r' || a == '\n')
			a = f.get();
		while (a != ':' && a != '\n') {
			line = line + a;
			a = f.get();
			if (a == EOF) {
				f.close();
				return;
			}
		}
		key = line;
		status[key].defined = true;
		status[key].name = line;
		while (true) {
			if (a == '\n')
				break;
			while (a == ' ' || a == ':') {
				a = f.get();
			}
			line = "";
			while (a != ' ' && a != ':' && a != '\n') {
				line = line + a;
				a = f.get();
			}
			if (line == "NONVOLATILE")
				status[key].nonvolatile = true;
			else if (line == "CHANCE")
				status[key].chance = true;
			else if (line == "SINGLETON")
				status[key].singleton = true;
			else if (line == "SPECIALCASE")
				status[key].specialcase = true;
		}
		while (a != '\n') {
			a = f.get();
		}
	}
	f.close();
}

void engine::init_blocking() {
	string line;
	ifstream f("../resources/data/blocking-tiles.dat");
	while (f.is_open()) {
		while (std::getline(f, line)) {
			blocking[stoi(line)] = true;
		}
		f.close();
	}
}

void engine::init_npc_blocking() {
	string line;
	ifstream f("../resources/data/npc-blocking-tiles.dat");
	while (f.is_open()) {
		while (std::getline(f, line)) {
			npc_blocking[stoi(line)] = true;
		}
		f.close();
	}
}

void engine::init_jumpdown() {
	string line;
	ifstream f("../resources/data/jumpdown-tiles.dat");
	while (f.is_open()) {
		while (std::getline(f, line)) {
			jumpdown[stoi(line)] = true;
		}
		f.close();
	}
}

void engine::init_jumpright() {
	string line;
	ifstream f("../resources/data/jumpright-tiles.dat");
	while (f.is_open()) {
		while (std::getline(f, line)) {
			jumpright[stoi(line)] = true;
		}
		f.close();
	}
}

void engine::init_jumpleft() {
	string line;
	ifstream f("../resources/data/jumpleft-tiles.dat");
	while (f.is_open()) {
		while (std::getline(f, line)) {
			jumpleft[stoi(line)] = true;
		}
		f.close();
	}
}

void engine::init_encounter_tiles() {
	string line;
	ifstream f("../resources/data/encounter-tiles.dat");
	while (f.is_open()) {
		while (std::getline(f, line)) {
			encounter_tile[stoi(line)] = true;
		}
		f.close();
	}
}

void engine::init_swimming() {
	string line;
	ifstream f("../resources/data/water-tiles.dat");
	while (f.is_open()) {
		while (std::getline(f, line)) {
			water[stoi(line)] = true;
		}
		f.close();
	}
}

void engine::init_exp() {
	string line;
	ifstream f("../resources/data/exp.dat");
	while (f.is_open()) {
		while (std::getline(f, line)) {
			level_to_exp.push_back(stoi(line));
		}
		f.close();
	}
}

void engine::init_special() {
	string line;
	ifstream f("../resources/data/special.dat");
	while (f.is_open()) {
		while (std::getline(f, line)) {
			special_case[line] = true;
		}
		f.close();
	}
}

void engine::init_types() {
	string line;
	ifstream f("../resources/data/types.dat", ios::binary);
	char a = 1;
	while (f.is_open()) {
		while (a && a != EOF) {
			a = f.get();
			if (a != ':')
				line = line + a;
			if (a == ':') {
				types[line];
				while (a != '\n' && a != 0) {
					a = f.get();
				}
				line = "";
				continue;
			}
		}
		f.close();
	}

	for (type_iter it1 = types.begin(); it1 != types.end(); it1++) {
		for (type_iter it2 = types.begin(); it2 != types.end(); it2++) {
			types[it1->first][it2->first] = 1.0f;
		}
	}

	ifstream f2("../resources/data/types.dat", ios::binary);
	a = 1;
	string attack, defense, mul;
	while (f2.is_open()) {
		while (a && a != EOF) {
			a = f2.get();
			if (a == '\r')
				a = f2.get();
			if (a == '\n')
				a = f2.get();
			if (a != ':')
				attack = attack + a;
			if (a == ':') {
				while (a == ':' || a == ' ') {
					a = f2.get();
				}
				while (true) {
					defense = "";
					mul = "";
					while (a != 'x') {
						defense = defense + a;
						a = f2.get();
					}
					a = f2.get();
					while (a != ' ' && a != '\r' && a != '\n') {
						mul = mul + a;
						a = f2.get();
					}
					if (mul == "2") {
						types[attack][defense] = 2.0f;
					}
					else if (mul == "0.5") {
						types[attack][defense] = 0.5f;
					}
					else if (mul == "0") {
						types[attack][defense] = 0.0f;
					}
					while (a == ' ')
						a = f2.get();
					if (a == '\n' || a == '\r')
						break;
				}
				attack = "";
				continue;
			}
		}
		f2.close();
	}
}

int engine::weightedrand() {
	int i = int(random(0.0, 8.0));
	switch (i) {
	case 0:
	case 1:
	case 2:
		return 0;
	case 3:
	case 4:
	case 5:
		return 1;
	case 6:
		return 2;
	case 7:
		return 3;
	default:
		return 0;
	}
}

void engine::handle_teleport() {
	for (unsigned i = 0; i < levels[mc.loc.level].teleport.size(); ++i) {
		if (levels[mc.loc.level].teleport[i].first.allow_player) {
			if (mc.loc.x == levels[mc.loc.level].teleport[i].first.x && mc.loc.y == levels[mc.loc.level].teleport[i].first.y) {
				transition.lock();
				mc.loc.x = levels[mc.loc.level].teleport[i].second.x;
				mc.loc.y = levels[mc.loc.level].teleport[i].second.y;
				mc.loc.level = levels[mc.loc.level].teleport[i].second.level;
				for (unsigned j = 0; j < levels[mc.loc.level].characters.size(); ++j) {
					levels[mc.loc.level].characters[j].dir = levels[mc.loc.level].characters[j].origin_dir;
					levels[mc.loc.level].characters[j].loc = levels[mc.loc.level].characters[j].origin;
				}
				transition.unlock();
				break;
			}
		}
	}
}

int engine::get_tile(double y, double x) {
	return levels[mc.loc.level].data[int(y)][int(x)];
}

void engine::input(bool up, bool down, bool left, bool right, bool select, bool start, bool confirm, bool cancel) {
	if (menus.size() == 0) {
		player_up = up;
		player_down = down;
		player_left = left;
		player_right = right;
		player_select = select;
		player_start = start;
		player_confirm = confirm;
		player_cancel = cancel;
	}
	else {
		menus[menus.size() - 1]->input(up, down, left, right, select, start, confirm, cancel);
	}
}

void engine::player_input(bool up, bool down, bool left, bool right, bool select, bool start, bool confirm, bool cancel) {
	// TODO:  Animations
	location l = mc.loc;
	ahead = l;
	ahead2 = l;
	if (left) {
		l.x -= 1.0;
		mc.dir = LEFT;
	}
	if (right) {
		l.x += 1.0;
		mc.dir = RIGHT;
	}
	if (up) {
		l.y -= 1.0;
		mc.dir = UP;
	}
	if (down) {
		l.y += 1.0;
		mc.dir = DOWN;
	}
	switch (mc.dir) {
	case UP:
		ahead.y -= 1.0;
		ahead2.y -= 2.0;
		break;
	case DOWN:
		ahead.y += 1.0;
		ahead2.y += 2.0;
		break;
	case LEFT:
		ahead.x -= 1.0;
		ahead2.x -= 2.0;
		break;
	case RIGHT:
		ahead.x += 1.0;
		ahead2.x += 2.0;
		break;
	}
	if (l.y == -1.0)
		return;
	if (l.y >= levels[mc.loc.level].data.size())
		return;
	if (l.x == -1.0)
		return;
	if (l.x >= levels[mc.loc.level].data[int(l.y)].size())
		return;
	if (blocking[get_tile(l.y, l.x)]) {
		if (down && jumpdown[get_tile(l.y, l.x)]) {}
		else if (right && jumpright[get_tile(l.y, l.x)]) {}
		else if (left && jumpleft[get_tile(l.y, l.x)]) {}
		else
			return;
	}
	if (water[get_tile(l.y, l.x)] && !has_move_in_party(string("SURF"))) {
		return;
	}
	if (water[get_tile(l.y, l.x)] && has_move_in_party(string("SURF"))) {
		if (!water[get_tile(mc.loc.y, mc.loc.x)]) {
			mc.movement = string("seal");
		}
	}
	if (mc.movement == string("seal") && !water[get_tile(l.y, l.x)]) {
		mc.movement = string("player");
	}
	if (down && jumpdown[get_tile(l.y, l.x)])
		l.y += 1.0;
	if (right && jumpright[get_tile(l.y, l.x)])
		l.x += 1.0;
	if (left && jumpleft[get_tile(l.y, l.x)])
		l.x -= 1.0;
	for (unsigned i = 0; i < levels[mc.loc.level].characters.size(); ++i) {
		if (!mc.active[levels[mc.loc.level].characters[i].name])
			continue;
		if (levels[mc.loc.level].characters[i].incorporeal)
			continue;
		if (l.x == levels[mc.loc.level].characters[i].loc.x && l.y == levels[mc.loc.level].characters[i].loc.y) {
			if (levels[mc.loc.level].characters[i].pushable && has_move_in_party(string("STRENGTH"))) {
				if (ahead2.x >= 0.0 && ahead2.y >= 0.0 && int(ahead2.y) < levels[mc.loc.level].data.size() && int(ahead2.x) < levels[mc.loc.level].data[int(ahead2.y)].size() && !blocking[get_tile(ahead2.y, ahead2.x)] && !npc_blocking[get_tile(ahead2.y, ahead2.x)]) {
					bool npc_in_way = false;
					for (unsigned j = 0; j < levels[mc.loc.level].characters.size(); ++j) {
						if (mc.active[levels[mc.loc.level].characters[j].name]) {
							if (ahead2.x == levels[mc.loc.level].characters[j].loc.x && ahead2.y == levels[mc.loc.level].characters[j].loc.y) {
								npc_in_way = true;
								break;
							}
						}
					}
					if (npc_in_way) {
						return;
					}
					else {
						levels[mc.loc.level].characters[i].loc.x = ahead2.x;
						levels[mc.loc.level].characters[i].loc.y = ahead2.y;
						if (levels[mc.loc.level].characters[i].teleportable) {
							for (unsigned j = 0; j < levels[mc.loc.level].teleport.size(); ++j) {
								if (levels[mc.loc.level].teleport[j].first.allow_npcs) {
									if (levels[mc.loc.level].characters[i].loc.x == levels[mc.loc.level].teleport[j].first.x && levels[mc.loc.level].characters[i].loc.y == levels[mc.loc.level].teleport[j].first.y) {
										mc.interaction[levels[mc.loc.level].characters[i].name] = levels[mc.loc.level].characters[i].interactions.size() - 1;
										do_interaction(levels[mc.loc.level].characters[i]);
										break;
									}
								}
							}
						}
						return;
					}
				}
				else {
					return;
				}
			}
			else {
				return;
			}
		}
	}
	if (confirm) {
		interact = true;
	}
	if (start) {
		open_menu = true;
	}
	if (mc.loc.x != l.x || mc.loc.y != l.y) {
		mc.loc = l;
		update_level();
		if (encounter_tile[get_tile(mc.loc.y, mc.loc.x)]) {
			if (levels[mc.loc.level].encounters.size() > 0) {
				if (random(0.0, 187.5) < 8.5) {
					if (levels[mc.loc.level].mega_encounters.size() > 0 && random(0.0, 20.0) >= 19.0) {
						int choice = int(random(0.0, double(levels[mc.loc.level].mega_encounters.size())));
						int choice2 = int(random(0.0, double(levels[mc.loc.level].mega_encounters.size())));
						encounter = std::to_string(levels[mc.loc.level].mega_encounters[choice]) + string("-") + std::to_string(levels[mc.loc.level].mega_encounters[choice2]);
						int l = int(random(levels[mc.loc.level].mega_level_range.first, levels[mc.loc.level].mega_level_range.second + 1));
						encounter_level = l;
					}
					else {
						int choice = int(random(0.0, double(levels[mc.loc.level].encounters.size())));
						int choice2 = int(random(0.0, double(levels[mc.loc.level].encounters.size())));
						encounter = std::to_string(levels[mc.loc.level].encounters[choice]) + string("-") + std::to_string(levels[mc.loc.level].encounters[choice2]);
						int l = int(random(levels[mc.loc.level].level_range.first, levels[mc.loc.level].level_range.second + 1));
						encounter_level = l;
					}
				}
			}
		}
		if (water[get_tile(mc.loc.y, mc.loc.x)]) {
			if (levels[mc.loc.level].water_encounters_super.size() > 0) {
				if (random(0.0, 187.5) < 8.5) {
					bool first = int(random(0.0, 2.0)) == 1;
					vector<int> combination;
					vector<int> base;
					for (unsigned i = 0; i < levels[mc.loc.level].encounters.size(); ++i) {
						combination.push_back(levels[mc.loc.level].encounters[i]);
					}
					for (unsigned i = 0; i < levels[mc.loc.level].water_encounters_super.size(); ++i) {
						combination.push_back(levels[mc.loc.level].water_encounters_super[i]);
						base.push_back(levels[mc.loc.level].water_encounters_super[i]);
					}
					int part1 = combination[int(random(0.0, combination.size()))];
					int part2 = base[int(random(0.0, base.size()))];
					int l = int(random(levels[mc.loc.level].level_range.first, levels[mc.loc.level].level_range.second + 1));
					encounter_level = l;
					if (first) {
						encounter = to_string(part1) + string("-") + to_string(part2);
					}
					else {
						encounter = to_string(part2) + string("-") + to_string(part1);
					}
				}
			}
		}
	}
}

void engine::create_menu(string s, string choice, string text_override, string followup_override) {
	menu* m = new menu;
	menus.push_back(m);
	menus[menus.size() - 1]->create_menu(s, choice, text_override, followup_override);
	menus[menus.size() - 1]->push_menu();
}

vector<int> engine::do_alert(string s) {
	return do_menu(string("ALERT"), s);
}

vector<int> engine::do_menu(string menu, string choice, string text_override, string followup_override) {
	vector<int> out;
	create_menu(menu, choice, text_override, followup_override);
	out = menus[menus.size() - 1]->main();
	delete menus[menus.size() - 1];
	menus.erase(menus.end() - 1);
	return out;
}

void engine::update_level() {
	m.lock();
	g.draw_list.clear();
	draw_level();
	draw_characters();
	m.unlock();
}

void engine::draw_level() {
	float xp, xl, yp, yl;
	string curr_level = mc.loc.level;
	double curr_x = mc.loc.x;
	double curr_y = mc.loc.y;
	level* l = &(levels[curr_level]);
	unsigned maxy = min(int(l->data.size()), max(int(curr_y + 5.0), 0));
	for (unsigned y = max(0, unsigned(curr_y - 4.0)); y < maxy; ++y) {
		unsigned maxx = min(int(l->data[y].size()), max(int(curr_x + 6.0), 0));
		for (unsigned x = max(0, unsigned(curr_x - 5.0)); x < maxx; ++x) {
			xp = -1.0f + (float(x) / 5.0f) - ((curr_x - 4.5f) / 5.0f);
			yp = (-float(y) / 4.5f) - (0.5f / 4.5f) + (curr_y / 4.5f);
			xl = 1.0f / 5.0f;
			yl = 1.0 / 4.5f;
			if (xp < -1.0f && xp + xl < -1.0f)
				continue;
			if (yp < -1.0f && yp + yl < -1.0f)
				continue;
			if (xp > 1.0f && xp + xl > 1.0f)
				continue;
			if (yp > 1.0f && yp + yl > 1.0f)
				continue;
			g.push_quad(xp, yp, xl, yl, g.tiles[l->data[y][x]]);
		}
	}
	for (unsigned i = 0; i < l->neighbors.size(); ++i) {
		level* n = &(levels[l->neighbors[i].level]);
		unsigned maxy = min(int(levels[levels[curr_level].neighbors[i].level].data.size()), max(int(curr_y - levels[curr_level].neighbors[i].y + 5.0), 0));
		for (unsigned y = max(0, unsigned(curr_y - levels[curr_level].neighbors[i].y - 4.0)); y < maxy; ++y) {
			unsigned maxx = min(int(levels[levels[curr_level].neighbors[i].level].data[y].size()), max(int(curr_x - levels[curr_level].neighbors[i].x + 6.0), 0));
			for (unsigned x = max(0, unsigned(curr_x - levels[curr_level].neighbors[i].x - 5.0)); x < maxx; ++x) {
				xp = -1.0f + (float(x + levels[curr_level].neighbors[i].x) / 5.0f) - ((curr_x - 4.5f) / 5.0f);
				yp = (-float(y + levels[curr_level].neighbors[i].y) / 4.5f) - (0.5f / 4.5f) + (curr_y / 4.5f);
				xl = 1.0f / 5.0f;
				yl = 1.0 / 4.5f;
				if (xp < -1.0f && xp + xl < -1.0f)
					continue;
				if (yp < -1.0f && yp + yl < -1.0f)
					continue;
				if (xp > 1.0f && xp + xl > 1.0f)
					continue;
				if (yp > 1.0f && yp + yl > 1.0f)
					continue;
				g.push_quad(xp, yp, xl, yl, g.tiles[levels[levels[curr_level].neighbors[i].level].data[y][x]]);
			}
		}
	}
}

void engine::draw_characters() {
	string curr_level = mc.loc.level;
	double curr_x = mc.loc.x;
	double curr_y = mc.loc.y;
	level* l = &(levels[curr_level]);
	for (unsigned i = 0; i < l->characters.size(); ++i) {
		character* c = &(l->characters[i]);
		if (!mc.active[c->name])
			continue;
		if (c->no_offset)
			g.push_quad((c->loc.x - (curr_x + 0.5)) / 5.0, (-0.5 - c->loc.y + curr_y) / 4.5f + 0.0, 1.0 / 5.0f, 1.0 / 4.5f, g.tex[c->image + string("-") + get_direction_string(c->dir) + string("-0.bmp")]);
		else
			g.push_quad((c->loc.x - (curr_x + 0.5)) / 5.0, (-0.5 - c->loc.y + curr_y) / 4.5f + 0.055, 1.0 / 5.0f, 1.0 / 4.5f, g.tex[c->image + string("-") + get_direction_string(c->dir) + string("-0.bmp")]);
	}
	for (unsigned j = 0; j < l->neighbors.size(); ++j) {
		level* n = &(levels[l->neighbors[j].level]);
		for (unsigned i = 0; i < n->characters.size(); ++i) {
			character* c = &(n->characters[i]);
			if (!mc.active[c->name])
				continue;
			if (c->no_offset)
				g.push_quad((levels[curr_level].neighbors[j].x + c->loc.x - (curr_x + 0.5)) / 5.0, (-levels[curr_level].neighbors[j].y - 0.5 - c->loc.y + curr_y) / 4.5f + 0.0, 1.0 / 5.0f, 1.0 / 4.5f, g.tex[c->image + string("-") + get_direction_string(c->dir) + string("-0.bmp")]);
			else
				g.push_quad((levels[curr_level].neighbors[j].x + c->loc.x - (curr_x + 0.5)) / 5.0, (-levels[curr_level].neighbors[j].y - 0.5 - c->loc.y + curr_y) / 4.5f + 0.055, 1.0 / 5.0f, 1.0 / 4.5f, g.tex[c->image + string("-") + get_direction_string(c->dir) + string("-0.bmp")]);
		}
	}
	g.push_quad(-0.1, -0.5 / 4.5 + 0.055, 1.0 / 5.0, 1.0 / 4.5, g.tex[mc.movement + string("-") + get_direction_string(mc.dir) + string("-0.bmp")]);
}

void engine::do_interaction(character& npc) {
	vector<int> choices;
	string s = npc.interactions[mc.interaction[npc.name]];
	string s2, s3;
	choices.clear();
	bool advance;
	advance = true;
	while (s != "") {
		if (s.find("MENU") == 0) {
			s.erase(0, s.find(":") + 1);
			s2 = s;
			s3 = s;
			s2.erase(s2.find(":"), s2.length());
			s3.erase(0, s3.find(":") + 1);
			if (s3.find("|") != -1)
				s3.erase(s3.find("|"), s3.length());
			choices = do_menu(s2, s3);
			s.erase(0, s2.length() + s3.length() + 1);
		}
		else if (s.find("NO_ADVANCE") == 0) {
			advance = false;
			s.erase(0, string("NO_ADVANCE").length() + 1);
			break;
		}
		else if (s.find("TELEPORT:") == 0) {
			s.erase(0, string("TELEPORT:").length());
			s2 = s;
			s2.erase(s2.find(" "), s2.size());
			s.erase(0, s.find(" ") + 1);
			string destination = s2;
			s2 = s;
			s2.erase(s2.find(" "), s2.size());
			s.erase(0, s.find(" ") + 1);
			int newx = stoi(s2);
			s2 = s;
			int newy = stoi(s2);
			mc.loc.level = destination;
			mc.loc.x = newx;
			mc.loc.y = newy;
			break;
		}
		else if (s.find("SET:") == 0) {
			s.erase(0, string("SET:").length());
			s2 = s;
			s.erase(0, s.find(":") + 1);
			s2.erase(s2.find(":"), s2.length());
			s3 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s3.erase(s3.find("|"), s3.length());
			}
			else {
				s = "";
			}
			mc.values[s2] = stoi(s3);
		}
		else if (s.find("GET:") == 0) {
			s.erase(0, string("GET:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			choices.clear();
			choices.push_back(e.mc.values[s2]);
		}
		else if (s.find("PLAY_SOUND:") == 0) {
			s.erase(0, string("PLAY_SOUND:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			se.play_sound(s2);
		}
		else if (s.find("PLAY_BLOCKING_SOUND:") == 0) {
			s.erase(0, string("PLAY_BLOCKING_SOUND:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			se.play_sound_blocking(s2);
		}
		else if (s.find("PLAY_MUSIC:") == 0) {
			s.erase(0, string("PLAY_MUSIC:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			se.play_music(s2);
		}
		else if (s.find("IS_PLAYER:") == 0) {
			s.erase(0, string("IS_PLAYER:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			choices.clear();
			if (s2 == "RIGHT") {
				if (npc.loc.x < mc.loc.x) {
					choices.push_back(1);
				}
				else {
					choices.push_back(0);
				}
			}
			if (s2 == "LEFT") {
				if (npc.loc.x > mc.loc.x) {
					choices.push_back(1);
				}
				else {
					choices.push_back(0);
				}
			}
			if (s2 == "UP") {
				if (npc.loc.y > mc.loc.y) {
					choices.push_back(1);
				}
				else {
					choices.push_back(0);
				}
			}
			if (s2 == "DOWN") {
				if (npc.loc.y < mc.loc.y) {
					choices.push_back(1);
				}
				else {
					choices.push_back(0);
				}
			}
		}
		else if (s.find("IN_INVENTORY:") == 0) {
			s.erase(0, string("IN_INVENTORY:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			choices.clear();
			if (in_inventory(s2))
				choices.push_back(1);
			else
				choices.push_back(0);
		}
		else if (s.find("HAS_MOVE:") == 0) {
			s.erase(0, string("HAS_MOVE:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			choices.clear();
			choices.push_back(has_move_in_party(s2));
		}
		else if (s.find("IS_SELECTION_SPECIES:") == 0) {
			s.erase(0, string("IS_SELECTION_SPECIES:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			if (s2 == mc.team[choices[0]].number) {
				choices.clear();
				choices.push_back(1);
			}
			else {
				choices.clear();
				choices.push_back(0);
			}
		}
		else if (s.find("TRADE_LOGIC:") == 0) {
			s.erase(0, string("TRADE_LOGIC:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			if (s2 == mc.team[choices[0]].number) {
				mc.team[choices[0]].defined = false;
				choices.clear();
				choices.push_back(1);
			}
			else {
				choices.clear();
				choices.push_back(0);
			}
		}
		else if (s.find("CAUGHT:") == 0) {
			s.erase(0, string("CAUGHT:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			std::map<string, bool>::iterator it = mc.caught.begin();
			int count = 0;
			while (it != mc.caught.end()) {
				if (it->second)
					count++;
				it++;
			}
			if (stoi(s2) <= count) {
				choices.clear();
				choices.push_back(1);
			}
			else {
				choices.clear();
				choices.push_back(0);
			}
		}
		else if (s.find("HAS_MONEY:") == 0) {
			s.erase(0, string("HAS_MONEY:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			choices.clear();
			if (mc.money >= stoi(s2))
				choices.push_back(1);
			else
				choices.push_back(0);
		}
		else if (s.find("HAS_COINS:") == 0) {
			s.erase(0, string("HAS_COINS:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			choices.clear();
			if (mc.coins >= stoi(s2))
				choices.push_back(1);
			else
				choices.push_back(0);
		}
		else if (s.find("REMOVE_MONEY:") == 0) {
			s.erase(0, string("REMOVE_MONEY:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			mc.money -= stoi(s2);
		}
		else if (s.find("REMOVE_COINS:") == 0) {
			s.erase(0, string("REMOVE_COINS:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			mc.coins -= stoi(s2);
		}
		else if (s.find("GIVE_MONEY:") == 0) {
			s.erase(0, string("GIVE_MONEY:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			mc.money += stoi(s2);
		}
		else if (s.find("GIVE_COINS:") == 0) {
			s.erase(0, string("GIVE_COINS:").length());
			s2 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			mc.coins += stoi(s2);
		}
		else if (s.find("ADVANCE") == 0) {
			s.erase(0, string("ADVANCE:").length());
			s2 = s;
			if (s2.find("|") != -1) {
				s2.erase(s2.find("|"));
				mc.interaction[s2]++;
			}
			else {
				mc.interaction[s2]++;
				s = "";
			}
		}
		else if (s.find("RESET") == 0) {
			s.erase(0, string("RESET:").length());
			s2 = s;
			if (s2.find("|") != -1) {
				s2.erase(s2.find("|"));
				mc.interaction[s2] = 0;
			}
			else {
				mc.interaction[s2] = 0;
				s = "";
			}
		}
		else if (s.find("FUSION:") == 0) {
			s.erase(0, s.find(":") + 1);
			s2 = s;
			if (s2.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			if (s2 == "JUVENILE" || s2 == "MATURE") {
				unsigned i = 0;
				unsigned j = 0;
				int level = 5;
				string parent1, parent2;
				for (i = 0; i < STORAGE_MAX; ++i) {
					for (j = 0; j < STORAGE_MAX; ++j) {
						if (!mc.storage[i][j].defined)
							break;
					}
					if (!mc.storage[i][j].defined)
						break;
				}
				mc.box_number = i;
				do_alert("Please select the first POK{e-accent}MON to take a blood sample from.");
				vector<int> choice1 = do_menu(string("MON_SELECT"));
				if (choice1[choice1.size() - 1] != -1) {
					do_alert("Please select the second POK{e-accent}MON to take a blood sample from.");
					vector<int> choice2 = do_menu(string("MON_SELECT"));
					if (choice2[choice2.size() - 1] != -1) {
						parent1 = mc.team[choice1[choice1.size() - 1]].number;
						parent2 = mc.team[choice2[choice2.size() - 1]].number;
						parent1.erase(parent1.find("-"), parent1.length());
						parent2.erase(parent2.find("-"), parent2.length());
						if (s2 == "MATURE") {
							level = min(mc.team[choice1[choice1.size() - 1]].level, mc.team[choice2[choice2.size() - 1]].level);
						}
						if (s2 == "JUVENILE") {
							parent1 = parent1 + string("-") + parent1;
							parent2 = parent2 + string("-") + parent2;
							std::map<string, mon_template>::iterator it;
							bool found = true;
							while (found) {
								it = all_mon.begin();
								found = false;
								while (it != all_mon.end()) {
									for (unsigned k = 0; k < it->second.evolution.size(); ++k) {
										if (it->second.evolution[k].first == parent1) {
											parent1 = it->second.number;
											found = true;
											break;
										}
									}
									it++;
								}
							}
							found = true;
							while (found) {
								it = all_mon.begin();
								found = false;
								while (it != all_mon.end()) {
									for (unsigned k = 0; k < it->second.evolution.size(); ++k) {
										if (it->second.evolution[k].first == parent2) {
											parent2 = it->second.number;
											found = true;
											break;
										}
									}
									it++;
								}
							}
							parent1.erase(parent1.find("-"), parent1.length());
							parent2.erase(parent2.find("-"), parent2.length());
						}
						do_alert(string("These samples will create this POK{e-accent}MON."));
						make_mon(parent1 + string("-") + parent2, level, mc.storage[i][j]);
						mc.seen[parent1 + string("-") + parent2] = true;
						do_menu(string("STATS_STORAGE"), to_string(j));
						int total = 0;
						total += get_stat(mc.storage[i][j], HP);
						total += get_stat(mc.storage[i][j], ATTACK);
						total += get_stat(mc.storage[i][j], DEFENSE);
						total += get_stat(mc.storage[i][j], SPECIAL);
						total += get_stat(mc.storage[i][j], SPEED);
						total += all_mon[mc.storage[i][j].number].stats[0];
						total += all_mon[mc.storage[i][j].number].stats[1];
						total += all_mon[mc.storage[i][j].number].stats[2];
						total += all_mon[mc.storage[i][j].number].stats[3];
						total += all_mon[mc.storage[i][j].number].stats[4];
						float holder = float(total);
						holder /= 150.0f;
						holder *= holder;
						holder *= sqrt(holder);
						holder *= 150.0f;
						total = int(holder);
						if (total <= mc.money) {
							vector<int> choice3 = do_menu(string("ALERT_YES_NO_MONEY"), string("Creating this will cost $") + to_string(total) + string("."));
							if (choice3[choice3.size() - 1] == 0) {
								bool found = false;
								mc.money -= total;
								do_alert(string("{PLAYER_NAME} got ") + get_nickname(mc.storage[i][j]) + string("!"));
								mc.caught[parent1 + string("-") + parent2] = true;
								for (unsigned k = 0; k < 6; ++k) {
									if (!mc.team[k].defined) {
										found = true;
										mc.team[k] = mc.storage[i][j];
										mc.team[k].enemy = false;
										mc.team[k].wild = false;
										mc.storage[i][j].defined = false;
									}
								}
								if (!found) {
									mc.storage[i][j].enemy = false;
									mc.storage[i][j].wild = false;
									do_alert(get_nickname(mc.storage[i][j]) + string(" was sent to box ") + to_string(i + 1) + string("!"));
								}
							}
							else {
								mc.storage[i][j].defined = false;
							}
						}
						else {
							do_alert("You don't have enough money for this!");
							mc.storage[i][j].defined = false;
						}
					}
				}
			}
			else {
				do_alert(string("SCRIPT ERROR: Called FUSION with parameter: ") + s2);
			}
		}
		else if (s.find("SHOP:") == 0) {
			s.erase(0, s.find(":") + 1);
			s2 = s;
			if (s2.find("|") != -1) {
				s.erase(0, s.find("|"));
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			if (s2 == "USED_TMS") {
				// TODO: Rebalance TM prices.
				s2 = "";
				std::map<string, bool>::iterator it2 = mc.used_tms.begin();
				bool first = true;
				while (it2 != mc.used_tms.end()) {
					if (it2->second) {
						if (!first) {
							s2 = s2 + ",";
						}
						s2 = s2 + it2->first;
						first = false;
					}
					it2++;
				}
				if (s2 == "") {
					do_alert("It looks like you don't have any TMs to recharge!");
					advance = false;
					break;
				}
			}
			choices.clear();
			string holder = s2;
			vector<string> item_holder;
			while (holder.find(",") != -1) {
				string holder2 = holder;
				holder2.erase(holder2.find(","), holder2.length());
				holder.erase(0, holder.find(",") + 1);
				item_holder.push_back(holder2);
			}
			item_holder.push_back(holder);
			while (choices.size() == 0 || choices[0] != 2) {
				choices = do_menu(string("SHOP_FRAME"), s2); // TODO: Reopen on anything except cancel.
				choices = remove_cancels(choices);
				if (choices.size() > 1) {
					if (choices[0] == 0) { // BUY
						if (choices[choices.size() - 1] == 0) { // Bought
							int num = (mc.money / items[item_holder[choices[1]]].price) - choices[2];
							gain_item(item_holder[choices[1]], num, true);
							mc.money -= num * items[item_holder[choices[1]]].price;
						}
					}
					else if (choices[0] == 1) { // SELL
						if (choices[choices.size() - 1] == 0) { // Sold
							holder = get_item_count(string("ALL"), choices[1]);
							while (holder.find("}") != -1) {
								holder.erase(0, holder.find("}") + 1);
							}
							int num = stoi(holder) - choices[2];
							mc.money += num * items[get_item_name(string("ALL"), choices[1])].price / 2;
							remove_item(get_item_name(string("ALL"), choices[1]), num);
						}
					}
				}
			}
		}
		else if (s.find("MON_STORAGE") == 0) {
			s.erase(0, s.find(":") + 1);
			s2 = s;
			if (s2.find("|") != -1) {
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			choices.clear();
			while (choices.size() == 0 || choices[0] != 4) {
				choices = do_menu("MON_STORAGE");
				choices = remove_cancels(choices);
				if (choices.size() > 0) {
					if (choices[0] == 0) {
						if (choices[2] == 0) {
							if (get_team_size() == 6) {
								do_alert(string("There's no room in your team!"));
							}
							else {
								unsigned holder = get_team_size();
								mc.team[holder] = mc.storage[mc.box_number][choices[1]];
								mc.team[holder].wild = false;
								mc.team[holder].enemy = false;
								mc.storage[mc.box_number][choices[1]].defined = false;
								pack_team();
								pack_storage();
							}
						}
					}
					else if (choices[0] == 1) {
						if (choices[2] == 0) {
							unsigned count = 0;
							while (count < STORAGE_MAX && mc.storage[mc.box_number][count].defined) {
								count++;
							}
							if (get_team_size() == 1) {
								do_alert(string("You can't deposit your last POK{e-accent}MON!"));
							}
							else if (count == STORAGE_MAX) {
								do_alert(string("No room left in that box!")); // TODO: Test this
							}
							else {
								mc.storage[mc.box_number][count] = mc.team[choices[1]];
								mc.storage[mc.box_number][count].wild = false;
								mc.storage[mc.box_number][count].enemy = false;
								mc.team[choices[1]].defined = false;
								pack_team();
								pack_storage();
							}
						}
					}
					else if (choices[0] == 2) {
						if (choices[2] == 0) {
							unsigned choice = choices[1];
							string name = get_nickname(mc.storage[mc.box_number][choice]);
							do_alert(string("You will probably never see ") + get_nickname(mc.storage[mc.box_number][choice]) + string(" again."));
							choices = do_menu(string("ALERT_YES_NO"), string("Are you sure you want to do this?"));
							if (choices[0] == 0) {
								do_alert(string("{PLAYER_NAME} released ") + name + string("."));
								switch (int(random(0.0, 8.0))) {
								case 0:
									do_alert(name + string(" lived happily ever after."));
									break;
								case 1:
									do_alert(name + string(" left to explore the Safari Zone."));
									break;
								case 2:
									do_alert(name + string(" will always fondly remember its time with you."));
									break;
								case 3:
									do_alert(name + string(" started a family and used your training to keep them safe."));
									break;
								case 4:
									do_alert(name + string(" returned home to its family."));
									break;
								case 5:
									do_alert(name + string(" returned to the wild with newfound confidence."));
									break;
								case 6:
									do_alert(name + string(" still brags to its friends that it was trained by {PLAYER_NAME}."));
									break;
								case 7:
									do_alert(name + string(" never liked you anyway."));
									break;
								default:
									do_alert(name + string(" left the party."));
									break;
								}
								mc.storage[mc.box_number][choice].defined = false;
							}
							pack_storage();
							pack_team();
							choices.clear();
						}
					}
					else if (choices[0] == 3) {
						mc.box_number = (STORAGE_MAX - 1) - choices[1];
					}
				}
			}
		}
		else if (s.find("ITEM_STORAGE") == 0) {
			s.erase(0, s.find(":") + 1);
			s2 = s;
			if (s2.find("|") != -1) {
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			choices.clear();
			while (choices.size() == 0 || choices[0] != 2) {
				choices = do_menu("ITEM_STORAGE");
				choices = remove_cancels(choices);
				if (choices.size() > 0) {
					if (choices[0] == 0) { // withdraw
						string dep = get_item_storage_name(string("ALL"), choices[1]);
						string holder = get_item_storage_count(string("ALL"), choices[1]);
						holder.erase(0, holder.find("}") + 1);
						int diff = stoi(holder) - choices[2];
						gain_item(dep, diff, true);
						withdraw_item(dep, diff);
					}
					else if (choices[0] == 1) { // deposit
						string dep = get_item_name(string("ALL"), choices[1]);
						string holder = get_item_count(string("ALL"), choices[1]);
						holder.erase(0, holder.find("}") + 1);
						int diff = stoi(holder) - choices[2];
						remove_item(dep, diff);
						store_item(dep, diff);
					}
				}
			}
		}
		else if (s.find("GIVE_MON:") == 0) {
			s.erase(0, string("GIVE_MON:").length());
			s2 = s;
			s.erase(0, s.find(":") + 1);
			s2.erase(s2.find(":"), s2.length());
			int l = stoi(s2);
			s2 = s;
			if (s2.find("|") != -1) {
				s2.erase(s2.find("|"), s2.length());
			}
			else {
				s = "";
			}
			bool mon_created = false;
			for (unsigned i = 0; i < 6; ++i) {
				if (!mc.team[i].defined) {
					make_mon(s2, l, mc.team[i]);
					mc.team[i].wild = false;
					mc.team[i].enemy = false;
					// TODO: Nickname menu
					do_menu(string("ALERT"), mc.name + string(" got ") + all_mon[mc.team[i].number].name + string("!"));
					mc.seen[mc.team[i].number] = true;
					mc.caught[mc.team[i].number] = true;
					mon_created = true;
					break;
				}
			}
			if (!mon_created) {
				for (unsigned i = 0; i < STORAGE_MAX && !mon_created; ++i) {
					for (unsigned j = 0; j < STORAGE_MAX && !mon_created; ++j) {
						if (!mc.storage[i][j].defined) {
							make_mon(s2, l, mc.storage[i][j]);
							mc.storage[i][j].wild = false;
							mc.storage[i][j].enemy = false;
							// TODO: Nickname menu
							do_menu(string("ALERT"), mc.name + string(" got ") + all_mon[mc.storage[i][j].number].name + string("!"));
							mc.seen[mc.storage[i][j].number] = true;
							mc.caught[mc.storage[i][j].number] = true;
							mon_created = true;
							do_menu(string("ALERT"), all_mon[mc.storage[i][j].number].name + string(" was sent to box ") + to_string(i + 1) + string("."));
							break;
						}
					}
				}
				// TODO: Storage full
			}
		}
		else if (s.find("SET_FACE:") == 0) {
			s.erase(0, s.find(":") + 1);
			s2 = s;
			s.erase(0, s.find(":") + 1);
			s2.erase(s2.find(":"), s2.length());
			direction dir = DOWN;
			if (s.find("LEFT") == 0) {
				dir = LEFT;
			}
			else if (s.find("RIGHT") == 0) {
				dir = RIGHT;
			}
			else if (s.find("UP") == 0) {
				dir = UP;
			}
			else if (s.find("DOWN") == 0) {
				dir = DOWN;
			}
			if (s2 == "PLAYER") {
				mc.dir = dir;
			}
			else {
				map<string, level>::iterator it;
				for (it = levels.begin(); it != levels.end(); it++) {
					for (unsigned k = 0; k < it->second.characters.size(); ++k) {
						if (it->second.characters[k].name == s2) {
							it->second.characters[k].dir = dir;
						}
					}
				}
			}
			update_level();
		}
		else if (s.find("FACE") == 0) {
			s.erase(0, s.find(":") + 1);
			if (s.find("LEFT") == 0) {
				npc.dir = LEFT;
				s.erase(0, string("LEFT").length());
			}
			else if (s.find("RIGHT") == 0) {
				npc.dir = RIGHT;
				s.erase(0, string("RIGHT").length());
			}
			else if (s.find("UP") == 0) {
				npc.dir = UP;
				s.erase(0, string("UP").length());
			}
			else if (s.find("DOWN") == 0) {
				npc.dir = DOWN;
				s.erase(0, string("DOWN").length());
			}
			update_level();
		}
		else if (s.find("GIVE_ITEM:") == 0) {
			s.erase(0, string("GIVE_ITEM:").length());
			s2 = s;
			if (s2.find("|") != -1) {
				s2.erase(s2.find("|"), s2.length());
			}
			gain_item(s2);
			s.erase(0, s2.length());
		}
		else if (s.find("REMOVE_ITEM:") == 0) {
			s.erase(0, string("REMOVE_ITEM:").length());
			s2 = s;
			if (s2.find("|") != -1) {
				s2.erase(s2.find("|"), s2.length());
			}
			if (remove_item(s2)) {
				do_alert(string("{PLAYER_NAME} gave ") + s2 + string("!"));
			}
			s.erase(0, s2.length());
		}
		else if (s.find("MOVE:") == 0) {
			direction dir;
			s.erase(0, string("MOVE:").length());
			s2 = s;
			s.erase(0, s.find(":") + 1);
			s2.erase(s2.find(":"), s2.length());
			s3 = s;
			s.erase(0, s.find(",") + 1);
			s3.erase(s3.find(","), s3.length());
			int deltax = stoi(s3);
			int deltay = 0;
			s3 = s;
			if (s.find("|") != -1) {
				s.erase(0, s.find("|"));
				s3.erase(s3.find("|"), s3.length());
			}
			else {
				s = "";
			}
			deltay = stoi(s3);
			if (deltax < 0)
				dir = LEFT;
			else if (deltax > 0)
				dir = RIGHT;
			else if (deltay > 0)
				dir = DOWN;
			else
				dir = UP;
			if (s2.find("PLAYER") == 0) {
				mc.loc.x += double(deltax);
				mc.loc.y += double(deltay);
				mc.dir = dir;
			}
			else {
				map<string, level>::iterator it;
				for (it = levels.begin(); it != levels.end(); it++) {
					for (unsigned k = 0; k < it->second.characters.size(); ++k) {
						if (it->second.characters[k].name == s2) {
							it->second.characters[k].loc.x += double(deltax);
							it->second.characters[k].loc.y += double(deltay);
							it->second.characters[k].dir = dir;
						}
					}
				}
			}
			update_level();
		}
		else if (s.find("{") == 0) {
			int counter;
			vector<string> options;
			unsigned x = 0;
			counter = 1;
			s2 = "";
			for (x = 1; x < s.size(); ++x) {
				if (s[x] == '{') {
					counter++;
					if (counter == 1) {
						continue;
					}
				}
				if (s[x] == '}') {
					counter--;
					if (counter == 0) {
						options.push_back(s2);
						s2 = "";
						continue;
					}
				}
				if (counter == 0 && s[x] == '|')
					break;
				s2 += s[x];
			}
			s.erase(0, x);
			s = options[choices[choices.size() - 1]] + s;
			continue;
		}
		else if (s.find("FULL_HEAL") == 0) {
			full_heal();
			mc.last_center.level = mc.loc.level;
			mc.last_center.x = mc.loc.x;
			mc.last_center.y = mc.loc.y;
		}
		else if (s.find("DEACTIVATE:") == 0) {
			s2 = s;
			s2.erase(0, string("DEACTIVATE:").length());
			if (s2.find("|") != -1) {
				s2.erase(s2.find("|"), s2.length());
			}
			else
				s = "";
			map<string, level>::iterator it;
			for (it = levels.begin(); it != levels.end(); it++) {
				for (unsigned k = 0; k < it->second.characters.size(); ++k) {
					if (it->second.characters[k].name == s2) {
						mc.active[it->second.characters[k].name] = false;
					}
				}
			}
			update_level();
		}
		else if (s.find("ACTIVATE:") == 0) {
			s2 = s;
			s2.erase(0, string("ACTIVATE:").length());
			if (s2.find("|") != -1) {
				s2.erase(s2.find("|"), s2.length());
			}
			else
				s = "";
			map<string, level>::iterator it;
			for (it = levels.begin(); it != levels.end(); it++) {
				for (unsigned k = 0; k < it->second.characters.size(); ++k) {
					if (it->second.characters[k].name == s2) {
						mc.active[it->second.characters[k].name] = true;
					}
				}
			}
			update_level();
		}
		else if (s.find("!:") == 0) {
			int clear_point = g.draw_list.size();
			s2 = s;
			s2.erase(0, string("!:").length());
			if (s2.find("|") != -1) {
				s2.erase(s2.find("|"), s2.length());
			}
			else
				s = "";
			if (s2 == "PLAYER") {
				g.push_quad_load(-0.1f, (0.5f / 4.5f) + 0.0416f, 0.2f, 1.0f / 4.5f, string("../resources/images/speechbubble.png"));
			}
			else {
				map<string, level>::iterator it;
				for (it = levels.begin(); it != levels.end(); it++) {
					for (unsigned k = 0; k < it->second.characters.size(); ++k) {
						if (it->second.characters[k].name == s2) {
							g.push_quad_load((it->second.characters[k].loc.x - (mc.loc.x + 0.5)) / 5.0f, (0.5 - it->second.characters[k].loc.y + mc.loc.y) / 4.5f + 0.0416f, 0.2f, 1.0f / 4.5f, string("../resources/images/speechbubble.png"));
						}
					}
				}
			}
			do_menu("BLANK", "BLANK");
			g.draw_list.erase(g.draw_list.begin() + clear_point, g.draw_list.end());
		}
		else if (s.find("SOUND:") == 0) {
			// TODO: Implement sound playing
			if (s.find("|") != -1) {
				s.erase(0, s.find("|") + 1);
			}
			else {
				s = "";
			}
		}
		else if (s.find("MAP") == 0) {
			// TODO: Implement MAP
			if (s.find("|") != -1) {
				s.erase(0, s.find("|") + 1);
			}
			else {
				s = "";
			}
		}
		else if (s.find("DISABLE_FORCE") == 0) {
			if (s.find("|") == -1) {
				s = "";
			}
			npc.no_force = true;
		}
		else if (s.find("MOVE_TO_PLAYER") == 0) {
			if (s.find("|") == -1) {
				s = "";
			}
			if (npc.loc.x < mc.loc.x)
				npc.loc.x = mc.loc.x - 1.0;
			if (npc.loc.x > mc.loc.x)
				npc.loc.x = mc.loc.x + 1.0;
			if (npc.loc.y < mc.loc.y)
				npc.loc.y = mc.loc.y - 1.0;
			if (npc.loc.y > mc.loc.y)
				npc.loc.y = mc.loc.y + 1.0;
		}
		else if (s.find("WILD_BATTLE") == 0) {
			s2 = s;
			s2.erase(0, s2.find(":") + 1);
			if (s2.find("|") != -1) {
				s2.erase(s2.find("|"), s2.length());
			}
			s3 = s2;
			s2.erase(s2.find(":"), s2.length());
			s3.erase(0, s3.find(":") + 1);
			mc.enemy_selected = 0;
			make_mon(s2, stoi(s3), mc.enemy_team[mc.enemy_selected]);
			for (int i = 0; i < 6; ++i) {
				mc.team[i].hp_bar_index = 0;
				mc.team[i].exp_bar_index = 0;
			}
			if (battle()) {
				choices.push_back(1);
			}
			else {
				choices.push_back(0);
			}
			for (int i = 0; i < 6; ++i) {
				mc.team[i].hp_bar_index = 0;
				mc.team[i].exp_bar_index = 0;
			}
			encounter = "";
			if (team_KO()) {
				full_heal();
				mc.loc.level = mc.last_center.level;
				mc.loc.x = mc.last_center.x;
				mc.loc.y = mc.last_center.y;
				mc.money /= 2;
			}
		}
		else if (s.find("BATTLE") == 0) {
			s2 = s;
			s2.erase(0, s2.find(":") + 1);
			if (s2.find("|") != -1) {
				s2.erase(s2.find("|"), s2.length());
			}
			choices.clear();
			for (int i = 0; i < 6; ++i) {
				mc.team[i].hp_bar_index = 0;
				mc.team[i].exp_bar_index = 0;
			}
			if (battle(levels[mc.loc.level].trainers[s2])) {
				choices.push_back(1);
			}
			else {
				choices.push_back(0);
			}
			for (int i = 0; i < 6; ++i) {
				mc.team[i].hp_bar_index = 0;
				mc.team[i].exp_bar_index = 0;
			}
		}
		if (s.find("|") != -1)
			s.erase(0, s.find("|") + 1);
		if (!advance)
			break;
	}
	if (advance) {
		if (mc.interaction[npc.name] < npc.interactions.size() - 1) {
			mc.interaction[npc.name]++;
		}
	}
	if (team_KO()) {
		full_heal();
		mc.loc.level = mc.last_center.level;
		mc.loc.x = mc.last_center.x;
		mc.loc.y = mc.last_center.y;
		mc.money /= 2;
	}
}

void engine::main() {
	while (true) {
		if (player_up || player_down || player_left || player_right || player_select || player_start || player_confirm || player_cancel)
			player_input(player_up, player_down, player_left, player_right, player_select, player_start, player_confirm, player_cancel);
		player_up = false;
		player_down = false;
		player_left = false;
		player_right = false;
		player_select = false;
		player_start = false;
		player_confirm = false;
		player_cancel = false;
		handle_teleport();
		transition.lock();
		for (unsigned i = 0; i < levels[mc.loc.level].characters.size(); ++i) {
			if (!mc.active[levels[mc.loc.level].characters[i].name])
				continue;
			for (unsigned j = 0; !levels[mc.loc.level].characters[i].no_force && j < levels[mc.loc.level].characters[i].force_interactions.size(); ++j) {
				if (mc.loc.x == levels[mc.loc.level].characters[i].force_interactions[j].x + levels[mc.loc.level].characters[i].loc.x &&
					mc.loc.y == levels[mc.loc.level].characters[i].force_interactions[j].y + levels[mc.loc.level].characters[i].loc.y) {
					if (!levels[mc.loc.level].characters[i].nolook) {
						if (levels[mc.loc.level].characters[i].loc.x > mc.loc.x)
							mc.dir = RIGHT;
						else if (levels[mc.loc.level].characters[i].loc.x < mc.loc.x)
							mc.dir = LEFT;
						else if (levels[mc.loc.level].characters[i].loc.y > mc.loc.y)
							mc.dir = DOWN;
						else if (levels[mc.loc.level].characters[i].loc.y < mc.loc.y)
							mc.dir = UP;
					}
					ahead.x = levels[mc.loc.level].characters[i].loc.x;
					ahead.y = levels[mc.loc.level].characters[i].loc.y;
					interact = true;
				}
			}
		}
		update_level();
		if (open_menu) {
			vector<int> choices;
			int offset = 0;
			bool found = false;
			open_menu = false;
			if (!e.mc.values[string("DEX")]) {
				offset++;
			}
			for (unsigned i = 0; i < 6; ++i) {
				if (mc.team[i].defined) {
					found = true;
					break;
				}
			}
			if (!found)
				offset += 2;
			choices = do_menu(string("MAINMENU") + to_string(offset));
			choices = remove_cancels(choices);
			choices[0] += offset;
			if (choices[0] == 1) {
				if (choices.size() == 4) {
					if (choices[2] == 1) {
						mon holder;
						holder = mc.team[choices[1]];
						mc.team[choices[1]] = mc.team[choices[3]];
						mc.team[choices[3]] = holder;
						mc.team[choices[1]].wild = false;
						mc.team[choices[1]].enemy = false;
						mc.team[choices[3]].wild = false;
						mc.team[choices[3]].enemy = false;
					}
				}
			}
			if (choices[0] == 2) { // INVENTORY
				string o;
				if (!can_use_item(string("NONCOMBAT"), get_item_name(string("ALL"), choices[1])) || !use_item(string("ALL"), choices, o)) { // TODO: Correct message for Oak's parcel
					do_alert("OAK: This isn't the time to use that!");
				}
			}
			if (choices[0] == 4) { // SAVING
				if (choices[choices.size() - 1] == 0) {
					save_game();
					do_alert("{PLAYER_NAME} saved the game!");
				}
			}
		}
		else if (interact) {
			interact = false;
			for (unsigned i = 0; i < levels[mc.loc.level].characters.size(); ++i) {
				if (!mc.active[levels[mc.loc.level].characters[i].name])
					continue;
				if ((ahead.x == levels[mc.loc.level].characters[i].loc.x && ahead.y == levels[mc.loc.level].characters[i].loc.y) ||
					(levels[mc.loc.level].characters[i].far && (ahead2.x == levels[mc.loc.level].characters[i].loc.x && ahead2.y == levels[mc.loc.level].characters[i].loc.y))) {
					if (levels[mc.loc.level].characters[i].loc.x < mc.loc.x)
						levels[mc.loc.level].characters[i].dir = RIGHT;
					else if (levels[mc.loc.level].characters[i].loc.x > mc.loc.x)
						levels[mc.loc.level].characters[i].dir = LEFT;
					else if (levels[mc.loc.level].characters[i].loc.y > mc.loc.y)
						levels[mc.loc.level].characters[i].dir = UP;
					else if (levels[mc.loc.level].characters[i].loc.y < mc.loc.y)
						levels[mc.loc.level].characters[i].dir = DOWN;
					do_interaction(levels[mc.loc.level].characters[i]);
					break;
				}
			}
		}
		transition.unlock();
		if (alert != "") {
			do_menu(string("ALERT"), alert);
			alert = "";
		}
		if (encounter != "") {
			mc.enemy_selected = 0;
			make_mon(encounter, encounter_level, mc.enemy_team[mc.enemy_selected]);
			for (int i = 0; i < 6; ++i) {
				mc.team[i].hp_bar_index = 0;
				mc.team[i].exp_bar_index = 0;
			}
			battle();
			for (int i = 0; i < 6; ++i) {
				mc.team[i].hp_bar_index = 0;
				mc.team[i].exp_bar_index = 0;
			}
			encounter = "";
			if (team_KO()) {
				full_heal();
				mc.loc.level = mc.last_center.level;
				mc.loc.x = mc.last_center.x;
				mc.loc.y = mc.last_center.y;
				mc.money /= 2;
			}
		}
		if (menus.size() > 0) {
			menus[0]->main();
			menus.erase(menus.begin());
		}
	}
}

void engine::pack_team() {
	unsigned j;
	for (unsigned i = 0; i < 6; ++i) {
		if (!mc.team[i].defined) {
			for (j = i + 1; j < 6; ++j) {
				if (mc.team[j].defined) {
					mc.team[i] = mc.team[j];
					mc.team[j].defined = false;
					mc.team[i].wild = false;
					mc.team[i].enemy = false;
					break;
				}
			}
			if (j == 6) {
				break;
			}
		}
	}
}

void engine::pack_storage() {
	unsigned j;
	for (unsigned i = 0; i < STORAGE_MAX; ++i) {
		if (!mc.storage[mc.box_number][i].defined) {
			for (j = i + 1; j < STORAGE_MAX; ++j) {
				if (mc.storage[mc.box_number][j].defined) {
					mc.storage[mc.box_number][i] = mc.storage[mc.box_number][j];
					mc.storage[mc.box_number][j].defined = false;
					mc.storage[mc.box_number][i].wild = false;
					mc.storage[mc.box_number][i].enemy = false;
					break;
				}
			}
			if (j == STORAGE_MAX) {
				break;
			}
		}
	}
}

void engine::save_game() {
	ofstream f("../SAVE.dat");
	f << string("WARNING: Editing this file can easily corrupt game state.\n");
	f << string("NAME:") + mc.name;
	f << string("\nRIVAL_NAME:") + mc.rivalname;
	f << string("\nMOVEMENT:") + mc.movement;
	f << string("\nDIRECTION:");
	f << int(mc.dir);
	f << string("\nWINS:");
	f << mc.wins;
	f << string("\nLOSSES:");
	f << mc.losses;
	f << string("\nLOCATION:");
	f << mc.loc.level;
	f << string("|");
	f << mc.loc.x;
	f << string("|");
	f << mc.loc.y;
	f << string("\nLAST_CENTER:");
	f << mc.last_center.level;
	f << string("|");
	f << mc.last_center.x;
	f << string("|");
	f << mc.last_center.y;
	f << string("\nMONEY:");
	f << mc.money;
	f << string("\nCOINS:");
	f << mc.coins;
	f << string("\nREPEL:");
	f << mc.repel;
	std::map<string, unsigned>::iterator it = mc.interaction.begin();
	f << string("\nINTERACTIONS:");
	while (it != mc.interaction.end()) {
		f << string("\n");
		f << it->first;
		f << string("|");
		f << it->second;
		it++;
	}
	f << string("\nEND\nACTIVE:");
	std::map<string, bool>::iterator it2 = mc.active.begin();
	while (it2 != mc.active.end()) {
		f << string("\n");
		f << it2->first;
		f << string("|");
		if (it2->second)
			f << string("1");
		else
			f << string("0");
		it2++;
	}
	f << string("\nEND\nINVENTORY:");
	for (unsigned i = 0; i < mc.inventory.size(); ++i) {
		f << string("\n");
		f << mc.inventory[i].first;
		f << string("|");
		f << mc.inventory[i].second;
	}
	f << string("\nEND\nINVENTORY_STORAGE:");
	for (unsigned i = 0; i < mc.inventory_storage.size(); ++i) {
		f << string("\n");
		f << mc.inventory_storage[i].first;
		f << string("|");
		f << mc.inventory_storage[i].second;
	}
	f << string("\nEND\nSEEN:");
	it2 = mc.seen.begin();
	while (it2 != mc.seen.end()) {
		if (it2->second) {
			f << string("\n");
			f << it2->first;
		}
		it2++;
	}
	f << string("\nEND\nCAUGHT:");
	it2 = mc.caught.begin();
	while (it2 != mc.caught.end()) {
		if (it2->second) {
			f << string("\n");
			f << it2->first;
		}
		it2++;
	}
	f << string("\nEND\nUSED_TMS:");
	it2 = mc.used_tms.begin();
	while (it2 != mc.used_tms.end()) {
		if (it2->second) {
			f << string("\n");
			f << it2->first;
		}
		it2++;
	}
	f << string("\nEND\nVALUES:");
	it = mc.values.begin();
	while (it != mc.values.end()) {
		f << string("\n");
		f << it->first;
		f << string("|");
		f << it->second;
		it++;
	}
	f << string("\nEND\nTEAM:");
	for (unsigned i = 0; i < 6; ++i) {
		if (mc.team[i].defined) {
			f << string("\n");
			f << i;
			f << string("|");
			save_mon(f, mc.team[i]);
		}
	}
	f << string("\nEND\nSTORAGE:");
	for (unsigned i = 0; i < STORAGE_MAX; ++i) {
		for (unsigned j = 0; j < STORAGE_MAX; ++j) {
			if (mc.storage[i][j].defined) {
				f << string("\n");
				f << i;
				f << string("|");
				f << j;
				f << string("|");
				save_mon(f, mc.storage[i][j]);
			}
		}
	}
	f << string("\nEND");
	f << string("\nLEVEL:");
	std::map<string, level>::iterator l = levels.begin();
	while (l != levels.end()) {
		f << string("\n") + l->first;
		for (unsigned i = 0; i < l->second.characters.size(); ++i) {
			f << string("\n") << i << string("|") << int(l->second.characters[i].dir) << string("|") << int(l->second.characters[i].loc.x) << string("|") << int(l->second.characters[i].loc.y) << string("|");
			if (l->second.characters[i].no_force)
				f << string("1");
			else
				f << string("0");
		}
		f << string("\nEND");
		++l;
	}
	f << string("\nEND_LEVELS");
	f.close();
}

void engine::save_mon(ofstream& f, mon& m) {
	f << string("NUMBER:") << m.number << string("|NICKNAME:") << m.nickname << string("|HP:") << m.curr_hp << string("|EXP:") << m.exp << string("|LEVEL:") << m.level << string("|EV:");
	for (unsigned i = 0; i < SIZE; ++i) {
		f << m.EV[i] << string("|");
	}
	f << string("IV:");
	for (unsigned i = 0; i < SIZE; ++i) {
		f << m.IV[i] << string("|");
	}
	f << string("MOVES:");
	for (unsigned i = 0; i < 4; ++i) {
		if (moves[m.moves[i]].defined) {
			f << m.moves[i];
		}
		f << string("|");
	}
	f << string("PP:");
	for (unsigned i = 0; i < 4; ++i) {
		if (moves[m.moves[i]].defined) {
			f << m.pp[i];
		}
		f << string("|");
	}
	f << string("MAX_PP:");
	for (unsigned i = 0; i < 4; ++i) {
		if (moves[m.moves[i]].defined) {
			f << m.max_pp[i];
		}
		f << string("|");
	}
	f << string("STATUS:");
	for (unsigned i = 0; i < m.status.size(); ++i) {
		f << m.status[i];
		if (i < m.status.size() - 1) {
			f << string("|");
		}
	}
}

void engine::load_game() {
	ifstream f("../SAVE.dat");
	string line;
	string temp;
	if (!f.is_open()) {
		return;
	}
	while (std::getline(f, line)) {
		if (line.find("NAME:") == 0) {
			line.erase(0, line.find(":") + 1);
			mc.name = line;
		}
		else if (line.find("RIVAL_NAME:") == 0) {
			line.erase(0, line.find(":") + 1);
			mc.rivalname = line;
		}
		else if (line.find("MOVEMENT:") == 0) {
			line.erase(0, line.find(":") + 1);
			mc.movement = line;
		}
		else if (line.find("DIRECTION:") == 0) {
			line.erase(0, line.find(":") + 1);
			mc.dir = direction(stoi(line));
		}
		else if (line.find("WINS:") == 0) {
			line.erase(0, line.find(":") + 1);
			mc.wins = stoi(line);
		}
		else if (line.find("LOSSES:") == 0) {
			line.erase(0, line.find(":") + 1);
			mc.losses = stoi(line);
		}
		else if (line.find("LOCATION:") == 0) {
			line.erase(0, line.find(":") + 1);
			temp = line;
			temp.erase(temp.find("|"), temp.length());
			line.erase(0, line.find("|") + 1);
			mc.loc.level = temp;
			temp = line;
			temp.erase(temp.find("|"), temp.length());
			line.erase(0, line.find("|") + 1);
			mc.loc.x = stoi(temp);
			temp = line;
			mc.loc.y = stoi(temp);
		}
		else if (line.find("LAST_CENTER:") == 0) {
			line.erase(0, line.find(":") + 1);
			temp = line;
			temp.erase(temp.find("|"), temp.length());
			line.erase(0, line.find("|") + 1);
			mc.last_center.level = temp;
			temp = line;
			temp.erase(temp.find("|"), temp.length());
			line.erase(0, line.find("|") + 1);
			mc.last_center.x = stoi(temp);
			temp = line;
			mc.last_center.y = stoi(temp);
		}
		else if (line.find("MONEY:") == 0) {
			line.erase(0, line.find(":") + 1);
			mc.money = stoi(line);
		}
		else if (line.find("COINS:") == 0) {
			line.erase(0, line.find(":") + 1);
			mc.coins = stoi(line);
		}
		else if (line.find("REPEL:") == 0) {
			line.erase(0, line.find(":") + 1);
			mc.repel = stoi(line);
		}
		else if (line.find("INTERACTIONS:") == 0) {
			std::getline(f, line);
			while (line != "END") {
				temp = line;
				temp.erase(temp.find("|"), temp.length());
				line.erase(0, line.find("|") + 1);
				mc.interaction[temp] = stoi(line);
				std::getline(f, line);
			}
		}
		else if (line.find("ACTIVE:") == 0) {
			std::getline(f, line);
			while (line != "END") {
				temp = line;
				temp.erase(temp.find("|"), temp.length());
				line.erase(0, line.find("|") + 1);
				mc.active[temp] = stoi(line) == 0 ? false : true;
				std::getline(f, line);
			}
		}
		else if (line.find("INVENTORY:") == 0) {
			std::getline(f, line);
			while (line != "END") {
				pair<string, int> p;
				temp = line;
				temp.erase(temp.find("|"), temp.length());
				line.erase(0, line.find("|") + 1);
				p.first = temp;
				p.second = stoi(line);
				mc.inventory.push_back(p);
				std::getline(f, line);
			}
		}
		else if (line.find("INVENTORY_STORAGE:") == 0) {
			std::getline(f, line);
			while (line != "END") {
				pair<string, int> p;
				temp = line;
				temp.erase(temp.find("|"), temp.length());
				line.erase(0, line.find("|") + 1);
				p.first = temp;
				p.second = stoi(line);
				mc.inventory_storage.push_back(p);
				std::getline(f, line);
			}
		}
		else if (line.find("SEEN:") == 0) {
			std::getline(f, line);
			while (line != "END") {
				mc.seen[line] = true;
				std::getline(f, line);
			}
		}
		else if (line.find("CAUGHT:") == 0) {
			std::getline(f, line);
			while (line != "END") {
				mc.caught[line] = true;
				std::getline(f, line);
			}
		}
		else if (line.find("USED_TMS:") == 0) {
			std::getline(f, line);
			while (line != "END") {
				mc.used_tms[line] = true;
				std::getline(f, line);
			}
		}
		else if (line.find("VALUES:") == 0) {
			std::getline(f, line);
			while (line != "END") {
				temp = line;
				temp.erase(temp.find("|"), temp.length());
				line.erase(0, line.find("|") + 1);
				mc.values[temp] = stoi(line);
				std::getline(f, line);
			}
		}
		else if (line.find("TEAM:") == 0) {
			std::getline(f, line);
			while (line != "END") {
				temp = line;
				temp.erase(temp.find("|"), temp.length());
				line.erase(0, line.find("|") + 1);
				load_mon(line, mc.team[stoi(temp)]);
				std::getline(f, line);
			}
		}
		else if (line.find("STORAGE:") == 0) {
			std::getline(f, line);
			while (line != "END") {
				int i, j;
				temp = line;
				temp.erase(temp.find("|"), temp.length());
				line.erase(0, line.find("|") + 1);
				i = stoi(temp);
				temp = line;
				temp.erase(temp.find("|"), temp.length());
				line.erase(0, line.find("|") + 1);
				j = stoi(temp);
				load_mon(line, mc.storage[i][j]);
				std::getline(f, line);
			}
		}
		else if (line.find("LEVEL:") == 0) {
			std::getline(f, line);
			while (line != "END_LEVELS") {
				string key;
				key = line;
				std::getline(f, line);
				while (line != "END") {
					int i, dir, x, y;
					temp = line;
					temp.erase(temp.find("|"), temp.length());
					line.erase(0, line.find("|") + 1);
					i = stoi(temp);
					temp = line;
					temp.erase(temp.find("|"), temp.length());
					line.erase(0, line.find("|") + 1);
					dir = stoi(temp);
					temp = line;
					temp.erase(temp.find("|"), temp.length());
					line.erase(0, line.find("|") + 1);
					x = stoi(temp);
					temp = line;
					temp.erase(temp.find("|"), temp.length());
					line.erase(0, line.find("|") + 1);
					y = stoi(temp);
					levels[key].characters[i].dir = direction(dir);
					levels[key].characters[i].loc.x = x;
					levels[key].characters[i].loc.y = y;
					levels[key].characters[i].no_force = stoi(line) == 1;
					std::getline(f, line);
				}
				std::getline(f, line);
			}
		}
	}
	f.close();
}

void engine::load_mon(string s, mon& m) {
	string temp = s;
	temp.erase(0, temp.find(":") + 1);
	temp.erase(temp.find("|"), temp.length());
	m.number = temp;
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(0, temp.find(":") + 1);
	temp.erase(temp.find("|"), temp.length());
	m.nickname = temp;
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(0, temp.find(":") + 1);
	temp.erase(temp.find("|"), temp.length());
	m.curr_hp = stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(0, temp.find(":") + 1);
	temp.erase(temp.find("|"), temp.length());
	m.exp = stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(0, temp.find(":") + 1);
	temp.erase(temp.find("|"), temp.length());
	m.level = stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(0, temp.find(":") + 1);
	temp.erase(temp.find("|"), temp.length());
	m.EV[0] = stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.EV[1] = stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.EV[2] = stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.EV[3] = stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.EV[4] = stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(0, temp.find(":") + 1);
	temp.erase(temp.find("|"), temp.length());
	m.IV[0] = stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.IV[1] = stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.IV[2] = stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.IV[3] = stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.IV[4] = stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(0, temp.find(":") + 1);
	temp.erase(temp.find("|"), temp.length());
	m.moves[0] = temp;
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.moves[1] = temp;
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.moves[2] = temp;
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.moves[3] = temp;
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(0, temp.find(":") + 1);
	temp.erase(temp.find("|"), temp.length());
	m.pp[0] = temp == "" ? 0 : stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.pp[1] = temp == "" ? 0 : stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.pp[2] = temp == "" ? 0 : stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.pp[3] = temp == "" ? 0 : stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(0, temp.find(":") + 1);
	temp.erase(temp.find("|"), temp.length());
	m.max_pp[0] = temp == "" ? 0 : stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.max_pp[1] = temp == "" ? 0 : stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.max_pp[2] = temp == "" ? 0 : stoi(temp);
	s.erase(0, s.find("|") + 1);
	temp = s;
	temp.erase(temp.find("|"), temp.length());
	m.max_pp[3] = temp == "" ? 0 : stoi(temp);
	s.erase(0, s.find("|") + 1);
	s.erase(0, s.find(":") + 1);
	while (s != "") {
		temp = s;
		if (temp.find("|") == -1)
			s = "";
		else {
			temp.erase(temp.find("|"), temp.length());
			s.erase(0, s.find("|") + 1);
		}
		m.status.push_back(temp);
	}
	m.defined = true;
	m.wild = false;
	m.enemy = false;
}