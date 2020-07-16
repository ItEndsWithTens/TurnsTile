# Changelog
Changes listed according to the [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) standard. This project attempts to use [Semantic Versioning](https://semver.org/spec/v2.0.0.html), to the best of its author's ability.

## [Unreleased]
### Added
- Add 64-bit builds
- Add Avisynth+ Linux and macOS support

### Changed
- Build with Avisynth+ header

### Removed
- Remove Avisynth 2.5 support

## [0.3.2] 2013-04-10
### Fixed
- Fix tilesheet processing for interlaced clips

## [0.3.1] 2011-09-21
### Changed
- Recompile to remove SSE/SSE2 requirement and Visual C++ CRT dependency

## [0.3.0] 2011-05-06
### Added
- Add YV12 support
- Add CLUTer palettizing function

### Changed
- Expand lotile/hitile operation to clip-only mode

### Fixed
- Correct braindead inclusion guard mistake in TurnsTile.h
- Fix 'res' option to round values correctly

## [0.2.1] 2011-03-12
### Fixed
- Update auto calc for tile size, now properly accounts for minimum YUY2 width

## [0.2.0] 2011-03-10
### Added
- Add 'levels', 'lotile', 'hitile'
- Add auto calculation of default tile size

### Changed
- Change 'res' behavior to simulate bit depth adjustment

## [0.1.0] 2010-12-24
- Initial release
