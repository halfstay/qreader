# Regression images

`multi-small-qr.png` is an original, generated test image, covered by this
repository's MIT license. It contains three ZXing-generated QR symbols with
different sizes and slight rotations. Payloads are
`https://example.org/test-payload-number-0` through `-2`.

The middle symbol is only about 70 pixels wide. In version 1.1.0 it was detected
but failed to decode after perspective resampling, while reading its unwarped
pixels worked. The rectangle `(300, 316, 120, 123)` isolates this symbol. Tests
require successful decoding both in isolation and in the complete image.

`detection-seed-sensitive.png` is another original generated scene (MIT), with
four rotated and partly overlapping symbols plus square distractors. OpenCV
4.5.5's k-means++ localization gives different candidate counts for RNG seeds
`11 + 7421 * n`. It tests deterministic positions/status after earlier analyses;
its damaged candidates are not required to decode successfully.

The Sina platform-code image supplied during development remains in `.build/`
and is not distributed as a test fixture.

The core test also generates `large-window.png` in the build's fixture folder:
a 2154x1380 white window containing a 90-pixel QR symbol. The former whole-image
path detected zero candidates, while a local crop decoded successfully. Tests
cover tile overlap, overview coordinates, repeated payloads at separate positions,
and (on a sufficiently large desktop) actual window capture and clipboard pixels.
