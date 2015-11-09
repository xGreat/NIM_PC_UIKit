﻿#pragma once
#include "module/msglog/msglog.h"

#include "module/login/login_manager.h"
#include "module/service/user_service.h"

enum SessionEventType
{
	SET_DELETE		//删除会话项
};

namespace nim_comp
{
/** @class SessionItem
  * @brief 会话列表项的UI类，当有新的会话消息或用户操作时，负责作出界面上的改变
  * @copyright (c) 2015, NetEase Inc. All rights reserved
  * @author towik
  * @date 2015/1/1
  */
class SessionItem : public ui::ListContainerElement
{
public:
	SessionItem();
	virtual ~SessionItem();
	virtual void Init(const nim::SessionData &msg);

	bool Match(const UTF8String& search_key)
	{
		//std::wstring ws_search_key = nbase::UTF8ToUTF16(search_key);
		//ws_search_key = nbase::MakeLowerString(ws_search_key);
		//if (nick_name_.find(ws_search_key) != std::wstring::npos
		//	|| nick_name_full_spell_.find(search_key) != UTF8String::npos
		//	|| nick_name_simple_spell_.find(search_key) != UTF8String::npos)
		//{
		//	return true;
		//}
		return false;
	}
	bool GetIsTeam()
	{
		return type_ == nim::kNIMSessionTypeTeam;
	}

	/**
	* 更新该会话列表项的显示（除最近一条消息外）
	* @return void	无返回值
	*/
	void UpdateInfo();

	/**
	* 更新该会话列表项的最近一条消息
	* @param[in] msg 消息内容和信息
	* @return void	无返回值
	*/
	void UpdateMsg(const nim::SessionData &msg);
	void ClearMsg();
	long long GetMsgTime();
	
	// 未读消息条数相关操作
	int GetUnread();
	void SetUnread(int unread);
	void AddUnread();
	void ResetUnread();

	static void DeleteRecentSessionCb(nim::NIMResCode code, const nim::SessionData &result, int total_unread_counts);
	static void BatchStatusDeleteCb(nim::NIMResCode res_code, const std::string& uid, nim::NIMSessionType to_type);
private:
	void UpdateUnread();
	void PopupSessionItemMenu(POINT point);
	bool DelSessionItemMenuItemClick(ui::EventArgs* param);
private:
	bool OnDbClicked(ui::EventArgs* arg);
	bool OnSessionItemMenu(ui::EventArgs* arg);
private:
	void OnTeamNameChange(const nim::TeamInfo& team_info);
private:
	ui::Label*		label_name_;
	ui::Label*		label_msg_;
	ui::Label*		label_time_;
	ui::Box*		box_unread_;
	ui::Label*		label_unread_;

	std::string		id_;
	nim::NIMSessionType	type_;
	long long		msg_time_;
	long			unread_count_;
	AutoUnregister unregister_cb;
};

//清除未读数
void InvokeResetUnread(const std::string &id, nim::NIMSessionType type);
}