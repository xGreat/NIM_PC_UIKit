#include "nim_cpp_friend.h"
#include "nim_sdk_helper.h"
#include "nim_common_helper.h"

namespace nim
{
typedef void(*nim_friend_reg_changed_cb)(const char *json_extension, nim_friend_change_cb_func cb, const void *user_data);
typedef void(*nim_friend_request)(const char *accid, NIMVerifyType verify_type, const char *msg, const char *json_extension, nim_friend_opt_cb_func cb, const void *user_data);
typedef void(*nim_friend_delete)(const char *accid, const char *json_extension, nim_friend_opt_cb_func cb, const void *user_data);
typedef void(*nim_friend_update)(const char *friend_json, const char *json_extension, nim_friend_opt_cb_func cb, const void *user_data);
typedef void(*nim_friend_get_list)(const char *json_extension, nim_friend_get_list_cb_func cb, const void *user_data);

// 好友信息变化执行回调函数，不销毁该回调函数
static void CallbackFriendChange(NIMFriendChangeType type, const char *result_json, const char *json_extension, const void *user_data)
{
	if (user_data)
	{
		Friend::FriendChangeCallback* cb_pointer = (Friend::FriendChangeCallback*)user_data;
		if (*cb_pointer)
		{
			FriendChangeEvent change_event;
			change_event.type_ = type;
			change_event.content_ = PCharToString(result_json);
			PostTaskToUIThread(std::bind((*cb_pointer), change_event));
			//(*cb_pointer)(change_event);
		}
	}
}

// 执行完好友操作回调函数后销毁之
static void CallbackFriendOpt(int res_code, const char *json_extension, const void *user_data)
{
	if (user_data)
	{
		Friend::FriendOptCallback* cb_pointer = (Friend::FriendOptCallback*)user_data;
		if (*cb_pointer)
		{
			PostTaskToUIThread(std::bind((*cb_pointer), (NIMResCode)res_code));
			//(*cb_pointer)((NIMResCode)res_code);
		}
		delete cb_pointer;
	}
}

// 获取好友列表后执行回调函数，然后销毁该回调函数
static void CallbackGetFriendsList(int res_code, const char *result_json, const char *json_extension, const void *user_data)
{
	if (user_data)
	{
		Friend::GetFriendsListCallback* cb_pointer = (Friend::GetFriendsListCallback*)user_data;
		if (*cb_pointer)
		{
			std::list<FriendProfile> friends_profile;
			ParseFriendsProfile(PCharToString(result_json), friends_profile);
			PostTaskToUIThread(std::bind((*cb_pointer), (NIMResCode)res_code, friends_profile));
			//(*cb_pointer)((NIMResCode)res_code, friends_profile);
		}
		delete cb_pointer;
	}
}

static Friend::FriendChangeCallback* friend_change_cb = nullptr;
void Friend::RegChangeCb(const FriendChangeCallback &cb, const std::string& json_extension /* = "" */)
{
	delete friend_change_cb;
	if (cb)
	{
		friend_change_cb = new FriendChangeCallback(cb);
	}
	return NIM_SDK_GET_FUNC(nim_friend_reg_changed_cb)(json_extension.c_str(), &CallbackFriendChange, friend_change_cb);
}

bool Friend::Request(const std::string &accid, NIMVerifyType verify_type, const std::string &msg, const FriendOptCallback &cb, const std::string& json_extension /*= ""*/)
{
	if (accid.empty())
		return false;

	FriendOptCallback* friend_opt_cb = nullptr;
	if (cb)
	{
		friend_opt_cb = new FriendOptCallback(cb);
	}
	NIM_SDK_GET_FUNC(nim_friend_request)(accid.c_str(), verify_type, msg.c_str(), json_extension.c_str(), &CallbackFriendOpt, friend_opt_cb);

	return true;
}

bool Friend::Delete(const std::string &accid, const FriendOptCallback &cb, const std::string& json_extension /*= ""*/)
{
	if (accid.empty())
		return false;

	FriendOptCallback* friend_opt_cb = nullptr;
	if (cb)
	{
		friend_opt_cb = new FriendOptCallback(cb);
	}
	NIM_SDK_GET_FUNC(nim_friend_delete)(accid.c_str(), json_extension.c_str(), &CallbackFriendOpt, friend_opt_cb);

	return true;
}

bool Friend::Update(const FriendProfile &friend_profile, const FriendOptCallback &cb, const std::string& json_extension /*= ""*/)
{
	if (friend_profile.GetAccId().empty())
		return false;

	FriendOptCallback* friend_opt_cb = nullptr;
	if (cb)
	{
		friend_opt_cb = new FriendOptCallback(cb);
	}
	NIM_SDK_GET_FUNC(nim_friend_update)(friend_profile.ToJsonString().c_str(), json_extension.c_str(), &CallbackFriendOpt, friend_opt_cb);

	return true;
}

void Friend::GetList(const GetFriendsListCallback& cb, const std::string& json_extension)
{
	GetFriendsListCallback* get_friends_list_cb = nullptr;
	if (cb)
	{
		get_friends_list_cb = new GetFriendsListCallback(cb);
	}
	return NIM_SDK_GET_FUNC(nim_friend_get_list)(json_extension.c_str(), &CallbackGetFriendsList, get_friends_list_cb);
}

bool Friend::ParseFriendAddEvent(const FriendChangeEvent& change_event, FriendAddEvent& out_event)
{
	if (change_event.type_ != kNIMFriendChangeTypeRequest)
		return false;

	Json::Value values;
	Json::Reader reader;
	if (reader.parse(change_event.content_, values) && values.isObject())
	{
		out_event.accid_ = values["accid"].asString();
		out_event.add_type_ = (NIMVerifyType)values["type"].asUInt();
		out_event.msg_ = values["msg"].asString();
		return true;
	}

	return false;
}
	
bool Friend::ParseFriendDelEvent(const FriendChangeEvent& change_event, FriendDelEvent& out_event)
{
	if (change_event.type_ != kNIMFriendChangeTypeDel)
		return false;

	Json::Value values;
	Json::Reader reader;
	if (reader.parse(change_event.content_, values) && values.isObject())
	{
		out_event.accid_ = values["accid"].asString();
		return true;
	}

	return false;
}

bool Friend::ParseFriendProfileUpdateEvent(const FriendChangeEvent& change_event, FriendProfileUpdateEvent& out_event)
{
	if (change_event.type_ != kNIMFriendChangeTypeUpdate)
		return false;

	Json::Value values;
	Json::Reader reader;
	if (reader.parse(change_event.content_, values) && values.isObject())
	{
		ParseFriendProfile(values["info"], out_event.profile_);
		return true;
	}

	return false;
}

bool Friend::ParseFriendProfileSyncEvent(const FriendChangeEvent& change_event, FriendProfileSyncEvent& out_event)
{
	if (change_event.type_ != kNIMFriendChangeTypeSyncList)
		return false;

	return ParseFriendsProfile(change_event.content_, out_event.profiles_);
}


}
