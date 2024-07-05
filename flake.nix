{
  description = "Description for the project";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    pre-commit-hooks = {

      url = "github:cachix/pre-commit-hooks.nix";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.flake-utils.follows = "flake-utils";
    };

  };

  outputs = inputs@{ flake-parts, nixpkgs, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [ "x86_64-linux" ];
      perSystem = {  pkgs, ... }: {
        devShells.default = pkgs.clangStdenv.mkDerivation {
          name = "cpp project template";
          stdenv = pkgs.clangStdenv;
          src = "./.";
          nativeBuildInputs = with pkgs; [
            gtest
            doxygen
            gnumake
            clang-tools
            fmt
            (python311.withPackages (ps: with ps; [ ipython black compiledb ]))
          ];
        };

      };
      flake = {
        formatter.x86_64-linux = nixpkgs.legacyPackages.x86_64-linux.nixpkgs-fmt;
      };
    };
}
