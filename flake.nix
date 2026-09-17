{
  inputs = {
    nixpkgs.url      = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url  = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (system: let
      overlays = [ ];
      pkgs = import nixpkgs {
        inherit system overlays;
        config.allowUnfree = true;
      };
      stdenv = pkgs.llvmPackages_22.stdenv;
    in {
      devShells.default = pkgs.mkShell.override { inherit stdenv; } {
        buildInputs = [
          pkgs.llvmPackages_22.clang-tools
          pkgs.llvmPackages_22.lldb
          pkgs.llvmPackages_22.bintools
          pkgs.tokei
        ];

        shellHook = ''
            export PATH="${pkgs.llvmPackages_22.clang-tools}/bin:$PATH"
            export PATH="${pkgs.llvmPackages_22.bintools}/bin:$PATH"
            export LLDB_DEBUGSERVER_PATH="/Applications/Xcode.app/Contents/SharedFrameworks/LLDB.framework/Versions/A/Resources/debugserver"
        '';
      };
    }
  );
}
