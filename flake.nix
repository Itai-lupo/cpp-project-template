{
  description = "Description for the project";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";


    src = {
      url = "file:.?submodules=1";
      type = "git";
      flake = false; # not including this results in inifite recursion
    };
    pre-commit-hooks = {

      url = "github:cachix/pre-commit-hooks.nix";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.flake-utils.follows = "flake-utils";
    };

  };

  outputs = inputs@{ flake-parts, nixpkgs, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [ "x86_64-linux" ];
      perSystem = { pkgs, lib, ... }: rec {
        devShells.default = packages.default;

        packages.default = pkgs.clangStdenv.mkDerivation {
          name = "cpp_project_template";

          stdenv = pkgs.clangStdenv;
          src = lib.sourceByRegex ./. [
            "^src.*"
            "^tests.*"
            "^include.*"
            "^vendor.*"
            "^.scripts.*"
            "^Makefile"
          ];
          submodules = 1;
          doCheck = true;

          installPhase = ''
            ls -a
            mkdir -p $out/bin
            cp .output/logger.out $out/bin/$name
          '';

          buildPhase = ''
            make all
          '';

          nativeBuildInputs = with pkgs; [
            gdb
            doxygen
            gnumake
            clang-tools
            (python3.withPackages (ps: with ps; [ ipython black compiledb ]))
          ];

          buildInputs = with pkgs; [
            fmt
            gtest
          ];
        };
      };
      flake = {
        formatter.x86_64-linux = nixpkgs.legacyPackages.x86_64-linux.nixpkgs-fmt;
      };
    };
}
