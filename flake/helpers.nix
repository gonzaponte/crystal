{ pkgs, pkgs-new, nain4, self }:

{
   shell-shared = {
    QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs-new.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs-new.libsForQt5.qt5.qtbase.version}/plugins";

      shellHook = ''
        export CRYSTAL_LIB=$PWD/install/crystal/lib
        export LD_LIBRARY_PATH=$CRYSTAL_LIB:$LD_LIBRARY_PATH;
        export PKG_CONFIG_PATH=$CRYSTAL_LIB/pkgconfig:$PKG_CONFIG_PATH;
      '';
    };
}
