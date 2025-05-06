// リビジョン文字列自動設定
// ビルド前ステップでgitwcrevでこのテキストからWCREVをワーキングコピーのリビジョンを文字列に変換する。

#pragma once

 inline static const char* project_revision = "rev $WCREV$";
 inline static const char* project_url = "$WCURL$";
 inline static const char* project_branch = "$WCBRANCH$";
