{
  description = "Toolchain environment manager";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/23.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }: 
    flake-utils.lib.eachDefaultSystem (system:
      let 
        pkgs = nixpkgs.legacyPackages.${system}; 
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          name = "tcenv-0.1.0";
          buildInputs = [
            (pkgs.boost.override { enableShared = false; })
            pkgs.libarchive
            pkgs.gcc13
          ];
          nativeBuildInputs = [
            pkgs.cmake
            pkgs.pkg-config
            pkgs.ninja
          ];
          src = ./.;
        };
      }
    );
}
