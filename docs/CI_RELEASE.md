# CI and store releases

Public GitHub Actions on this repository only compile and test. They do not
have Apple signing secrets.

- `CI` runs macOS unit tests and an unsigned iOS compile on `main` pushes,
  pull requests, and manual dispatch.
- Signed macOS/iOS TestFlight uploads run from the private workflow
  `FA Editor store release`. That job checks out this source tree and uses
  Apple Distribution and App Store Connect credentials stored privately.
  Logs and artifacts for those runs stay on the private repository.

```sh
gh workflow run "FA Editor store release" -R teho1/ReminderApp --ref main \
  -f version=1.0 -f platform=ios -f submit_ios_review=false -f checkout_ref=main
```

Do not add Apple `.p12` or `.p8` material as secrets on this public
repository. Review CMake and script changes before merging; a signed
release builds this source on a runner that later loads the certificate.

Production App Store review is not started by a tag. Use that private
workflow with **Submit the iOS build for App Store review** after TestFlight
processing and iPad screenshots exist in App Store Connect. The iOS binary is
iPad-only (`UIDeviceFamily` 2), so App Store Connect does not require iPhone
screenshot sizes.
