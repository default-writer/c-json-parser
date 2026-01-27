#!/bin/bash

case "$(uname)" in
    "Darwin")
        echo "Unsupported Operating System: $(uname)"
        exit 1
        ;;
esac

if [[ "${BASHOPTS}" != *extdebug* ]]; then
    set -e
fi

err_report() {
    cd ${cwd}
    echo "ERROR: $0:$*"
    exit 8
}

if [[ "${BASHOPTS}" != *extdebug* ]]; then
    trap 'err_report $LINENO' ERR
fi

sudo apt update -y
sudo apt upgrade -y
sudo apt install -y lsb-release wget gpg gcc make cmake unzip

cwd=$(cd "$(dirname $(dirname "${BASH_SOURCE[0]}"))" &> /dev/null && pwd)
TOOLS_DIR="${cwd}/.tools"

mkdir -p "$TOOLS_DIR/llvm"

wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 21 all

if [[ ! -f "$TOOLS_DIR/llvm/lldb-dap" ]]; then
   sudo ln -s /usr/bin/lldb-dap-21 $TOOLS_DIR/llvm/lldb-dap
fi
rm ./llvm.sh

if [[ ! -f "$TOOLS_DIR/llvm/clang" ]]; then
  sudo ln -s /usr/bin/clang-21 $TOOLS_DIR/llvm/clang
fi

NINJA_VERSION="1.11.1"
NINJA_ZIP="ninja-linux.zip"
mkdir -p "$TOOLS_DIR/ninja"
wget -O "$TOOLS_DIR/ninja/$NINJA_ZIP" "https://github.com/ninja-build/ninja/releases/download/v$NINJA_VERSION/$NINJA_ZIP"
unzip -o "$TOOLS_DIR/ninja/$NINJA_ZIP" -d "$TOOLS_DIR/ninja"
rm "$TOOLS_DIR/ninja/$NINJA_ZIP"
chmod +x "$TOOLS_DIR//ninja/ninja"
echo "ninja installed in $TOOLS_DIR/ninja"

export PATH=$TOOLS_DIR:$PATH
export CLANG="$(dirname $(whereis clang | sed 's/.* //g'))"

grep -qxF '# ninja' $HOME/.bashrc || (tail -1 $HOME/.bashrc | grep -qxF '' || echo '' >> $HOME/.bashrc && echo '# ninja' >> $HOME/.bashrc)
grep -qxF 'export PATH='"$TOOLS_DIR/ninja"':$PATH' $HOME/.bashrc || echo 'export PATH='"$TOOLS_DIR/ninja"':$PATH' >> $HOME/.bashrc
grep -qxF '# llvm' $HOME/.bashrc || (tail -1 $HOME/.bashrc | grep -qxF '' || echo '' >> $HOME/.bashrc && echo '# llvm' >> $HOME/.bashrc)
grep -qxF 'export PATH='"$TOOLS_DIR/llvm"':$PATH' $HOME/.bashrc || echo 'export PATH='"$TOOLS_DIR/llvm"':$PATH' >> $HOME/.bashrc

. $HOME/.bashrc

[[ $SHLVL -eq 2 ]] && echo OK

cd "${cwd}"