#pragma once
#ifndef AUTH_H
#define AUTH_H
int auth_login(const char *username, const char *password, char out_token[65]);
int auth_register(const char *username, const char *password);
int auth_validate(const char *token);
void auth_logout(const char *token);
int auth_cookie_token(const char *cookie_header, char out[65]);
int auth_change_password(const char *token, const char *old_password, const char *new_password);
int auth_change_username(const char *token, const char *current_password, const char *new_username);
#endif
