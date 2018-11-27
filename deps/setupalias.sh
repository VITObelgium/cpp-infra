git remote add vcpkg-ports https://git.vito.be/scm/marvin/vcpkg-ports.git
git remote add infra https://git.vito.be/scm/marvin-geodynamix/infra.git

git config alias.vcpkgpull "subtree pull --squash  --prefix=deps/vcpkg vcpkg-ports master"
git config alias.infpull "subtree pull --squash  --prefix=deps/infra infra master"

git config alias.vcpkgpush "subtree push --prefix=deps/vcpkg vcpkg-ports master"
git config alias.infpush "subtree push --prefix=deps/infra infra master"
