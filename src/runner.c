#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <runner.h>
#include <memory.h>
#include <notify.h>
#include <joedog/boolean.h>
#include <joedog/defs.h>

struct RUNNER_T
{
  int     len;
  BOOLEAN runas;
  struct  group   grp;
  struct  passwd  pwd;
};

private BOOLEAN __set_user(RUNNER this, char *user);
private BOOLEAN __set_group(RUNNER this, char *group);

RUNNER
new_runner(char *user, char *group)
{
  RUNNER this;

  this = calloc(sizeof(struct RUNNER_T),1);
  this->len   = 16384;
  this->runas = TRUE;
  __set_user(this,  user);
  __set_group(this, group);
  return this;
}

void
runner_destroy(RUNNER this) {
  xfree(this);
  return;
} 

private BOOLEAN 
__set_user(RUNNER this, char *user)
{
  char          *buf;
  struct passwd *res;


  if (user == NULL) {
    this->runas = FALSE;
    return this->runas;
  }

  buf = xmalloc(this->len);

  if ((getpwnam_r(user, &this->pwd, buf, this->len, &res)) < 0) {
    switch(errno){
      case EINTR:           { NOTIFY(ERROR, "caught signal %s:%d",       __FILE__, __LINE__); break; }
      case EIO:             { NOTIFY(ERROR, "I/O error %s:%d",           __FILE__, __LINE__); break; }
      case EMFILE:          { NOTIFY(ERROR, "file table full %s:%d",     __FILE__, __LINE__); break; }
      case ENOMEM:          { NOTIFY(ERROR, "insufficient memory %s:%d", __FILE__, __LINE__); break; }
      case ERANGE:          { NOTIFY(ERROR, "insufficient buffer %s:%d", __FILE__, __LINE__); break; }
      default:              { NOTIFY(ERROR, "unknown user error %s:%d",  __FILE__, __LINE__); break; }
    } xfree(buf); return FALSE;
  }
  xfree(buf);
  return TRUE;
}

private BOOLEAN
__set_group(RUNNER this, char *group)
{
  char         *buf;
  struct group *res;


  if (group == NULL) {
     if (this->pwd.pw_uid > 0) {
       this->grp.gr_gid = this->pwd.pw_gid;
       return TRUE;
     }
     return FALSE;
   }

  buf = xmalloc(this->len);

  if ((getgrnam_r(group, &this->grp, buf, this->len, &res)) < 0) {
    switch(errno){
      case EINTR:           { NOTIFY(ERROR, "caught signal %s:%d",       __FILE__, __LINE__); break; }
      case EIO:             { NOTIFY(ERROR, "I/O error %s:%d",           __FILE__, __LINE__); break; }
      case EMFILE:          { NOTIFY(ERROR, "file table full %s:%d",     __FILE__, __LINE__); break; }
      case ENOMEM:          { NOTIFY(ERROR, "insufficient memory %s:%d", __FILE__, __LINE__); break; }
      case ERANGE:          { NOTIFY(ERROR, "insufficient buffer %s:%d", __FILE__, __LINE__); break; }
      default:              { NOTIFY(ERROR, "unknown user error %s:%d",  __FILE__, __LINE__); break; }
    } xfree(buf); return FALSE;
  }
  xfree(buf);
  return TRUE;
}

BOOLEAN
runas(RUNNER this)
{
  if (!this->runas) {
    return FALSE;
  }

  if (setgroups(0,0) != 0){
      NOTIFY(ERROR, "Dropping supplementary group privileges failed.");
      return FALSE;
  }
  if (setgid((long)this->grp.gr_gid) != 0) {
    NOTIFY(ERROR, "Unable to run as group: %s [%ld]", this->grp.gr_name, (long)this->grp.gr_gid);
    return FALSE;
  }
  if (setuid((long)this->pwd.pw_uid) != 0) {
    NOTIFY(ERROR, "Unable to run as user: %s [%ld]", this->pwd.pw_name, (long)this->pwd.pw_uid);
    return FALSE;
  } 
  return TRUE;
}

#if 0
int main() {
  RUNNER R = new_runner("apache", "nobody");
  printf("UID: %ld, GID: %ld\n", getuid(), getgid());
  runas(R);
  printf("UID: %ld, GID: %ld\n", getuid(), getgid());
  exit(0); 
}
#endif/*1|0*/
