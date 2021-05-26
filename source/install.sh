#!/usr/bin/env bash
NAME="server_fw"

DEF_FILE="/etc/default/$NAME"
LOG_FILE="/etc/rsyslog.d/$NAME.conf"
APP_DIR="/mnt/tftp"
SRCH_DIR="/mnt/backup"
BIN_DIR="bin"
SRC_DIR="source"
DAEMON_FILE_SRC="$BIN_DIR/$NAME"
DAEMON_FILE_DST="/usr/sbin/$NAME"
DAEMON_INIT_SRC="$SRC_DIR/daemon.init"
DAEMON_INIT_DST="/etc/init.d/$NAME"

do_make_inst_def(){
  if [ ! -f "$DAEMON_FILE_SRC" ]; then
    echo "[ERROR] No '$DAEMON_FILE_SRC' binary found (need make?)"
    exit 1
  fi
  if [ ! -d "$APP_DIR" ]; then
    echo "[INSTALL] Creating directory $APP_DIR" &&
    sudo mkdir -p "$APP_DIR"
    sudo chmod 0777 "$APP_DIR"
  fi

  if [ ! -d "$SRCH_DIR" ]; then
    echo "[INSTALL] Creating directory $SRCH_DIR" &&
    sudo mkdir -p "$SRCH_DIR"
    sudo chmod 0777 "$SRCH_DIR"
  fi

  echo "[INSTALL] Killing all $NAME instances"
  sudo service "$NAME" stop
  sleep 1
  sudo killall -s SIGHUP -q "$NAME" > /dev/null 2>&1
  sleep 1
  echo "[INSTALL] Copying daemon file $DAEMON_FILE_DST"
  sudo cp "$BIN_DIR/$NAME" "$DAEMON_FILE_DST"
  sudo chmod +x "$DAEMON_FILE_DST"
  
  echo "[INSTALL] Copying daemon init script $DAEMON_INIT_DST"
  sudo cp "$DAEMON_INIT_SRC" "$DAEMON_INIT_DST"
  sudo chmod +x "$DAEMON_INIT_DST"

  if [ ! -f "$LOG_FILE" ]; then
    echo "[INSTALL] Creating $NAME rsyslog config file"
  else
    echo "[INSTALL] Rewrite $NAME rsyslog config file"
  fi
  echo "if \$programname == '$NAME' then /var/log/$NAME.log" > "$LOG_FILE"
  echo "& stop" >>  "$LOG_FILE"
  echo "[INSTALL] Restart rsyslog daemon"
  sudo service rsyslog restart

  if [ ! -f "$DEF_FILE" ]; then
    echo "[INSTALL] Creating daemon config file"
    echo "IP=0.0.0.0:69" >> "$DEF_FILE"
    echo "SYSLOG=6" >> "$DEF_FILE"
    echo "ROOT_DIR=/mnt/tftp" >> "$DEF_FILE"
    echo "SERACH=/mnt/backup" >> "$DEF_FILE"
    echo "OWNER_USER=tftp" >> "$DEF_FILE"
    cat "$DEF_FILE"
  fi
}

do_make_autoservice(){
  sudo update-rc.d -f "$NAME" defaults
  sudo systemctl enable "$NAME"
  sudo systemctl daemon-reload
  echo "[INFO] Systemd $NAME configure autostart"
  echo "[INFO] Check daemon settings in $DEF_FILE"
}

do_remove_autostart(){
  sudo service server_fw stop
  sleep 1
  sudo update-rc.d -f "$DAEMON_INIT_DST" remove
  echo "[INFO] Systemd $NAME autostart disabled"
}

do_uninstall(){
  sudo killall -s SIGHUP -q "$NAME" > /dev/null 2>&1
  sleep 1
  do_remove_autostart
  [ -f "$DAEMON_INIT_DST" ] && echo "[UNINSTALL] Remove file $DAEMON_INIT_DST"  && sudo rm -f  "$DAEMON_INIT_DST"
  [ -f "$LOG_FILE"    ] && echo "[UNINSTALL] Remove file $LOG_FILE"     && sudo rm -f  "$LOG_FILE"
  [ -f "$DEF_FILE"    ] && echo "[UNINSTALL] Remove file $DEF_FILE"     && sudo rm -f  "$DEF_FILE"
  sudo service rsyslog restart
  sleep 1
  [ -f "$DAEMON_FILE_DST" ] && echo "[UNINSTALL] Remove file $DAEMON_FILE_DST"  && sudo rm -f  "$DAEMON_FILE_DST"
}

if [ ! $(id -u) -eq 0 ]; then
  echo "Need run as sudo!"
  exit 0
fi

# main()

action=$1
if [ -z "$action" ]; then
  echo "No silent options {all|allauto|noauto|remove} skip silent mode."
  echo "Interactive install '$NAME' options:"
  echo -e " [1] 'all'     Install defaults (copy files and make directories)"
  echo -e " [2] 'allauto' Install defaults and enable auto service with systemd"
  echo -e " [3] 'noauto'  Disabe auto service $NAME"
  echo -e " [4] 'remove'  Full remove $NAME"
  echo -n "Enter you choise [1-4] (recommended 2)? "
  read rpl
  case $rpl in
    1) 
      action="all"
      ;;
    2)
      action="allauto"
      ;;
    3)
      action="noauto"
      ;;
    4)
      action="remove"
      ;;
    *)
      action="nothing"
      ;;
  esac
fi

case $action in
  all) 
    do_make_inst_def
    ;;
  auto)
    do_make_autoservice
    sudo service $NAME start
    ;;
  allauto)
    do_make_inst_def
    do_make_autoservice
    sudo service $NAME start
    ;;
  noauto)
    do_remove_autostart
    ;;
  remove)
    do_uninstall
    ;;
  *)
    echo "Do nothing"
    ;;
esac

