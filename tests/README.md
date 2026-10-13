# tests

Host-side unit tests and policy checks for the FujiNet-PC build. ctest runs them at the end of
`./build.sh -p <TARGET>` and in the Build FujiNet-PC workflow.

## Layout
| File | Covers |
|---|---|
| `CMakeLists.txt` | One `add_executable` and `add_test` pair per test; each links only the unit under test, never the `fujinet` executable |
| `FujiBusPacketTests.cpp` | `lib/bus/rs232/FujiBusPacket.cpp` SLIP framing (`fujibuspacket_tests`) |
| `FnSanitizeTests.cpp` | `lib/fntext/fn_sanitize.cpp` entity decoding and UTF-8 folding (`fn_sanitize_tests`) |
| `FnxmlQueryTests.cpp` | `lib/fnxml/fnxml_query.cpp` XPath subset over tinyxml2 (`fnxml_query_tests`) |
| `CalendarTests.cpp` | `lib/utils/fn_time.cpp` and `lib/network-protocol/calendar_draft.cpp` (`calendar_tests`) |
| `QRCodeTests.cpp` | `lib/qrcode/qrcode.c` version and ECC selection (`qrcode_tests`) |
| `MailDraftTests.cpp` | `lib/network-protocol/mail_draft.cpp` RFC822 compose and reply (`mail_tests`) |
| `IMDImageTests.cpp`, `IMDFixture.h` | `lib/media/IMDImage.cpp` (`imdimage_tests`; built with `UNIT_TESTS` and `FNIO_IS_STDIO`) |
| `MediaTypeIMDTests.cpp` | `lib/media/rs232/diskTypeIMD.cpp` adapter (`mediatypeimd_tests`; built with `BUILD_RS232`) |
| `sioNetwork-DSTATS-test.cpp` | `sioNetwork` command recognition and DSTATS (`sio_dstats_tests`; ATARI target only) |
| `sioNetworkTests.cpp` | What `sioNetwork` sends to the bus for a failed CLOSE, its repeat and a failed READ (`sio_network_tests`; ATARI target only) |
| `sioNetworkStubs.cpp`, `sioNetworkStubs.h` | Link-time stubs shared by `sio_dstats_tests` and `sio_network_tests`; the `systemBus` stubs record into `bus` |
| `NQueryOutputModeTests.cpp`, `NParserReadTests.cpp` | `NParser` and `XMLParser` query output modes and short reads (`nquery_output_mode_tests`; ATARI target only) |
| `MgHttpCompletionProbe.cpp`, `mg_http_completion_test.py` | Real PC HTTP fixed-length/chunked completion, redirect connection isolation, two-request reuse and body/status/EOF (`mg_http_completion_tests`; POSIX hosts) |
| `HttpHeaderTests.cpp` | `lib/fn_esp_http_client/fn_http_header.cpp` header chunking (`http_header_tests`; skipped where the BSD queue header is missing) |
| `WebDavMethodsTests.cpp` | `lib/http/webdav/webdav_methods.h`: every method the WebDAV server handles is registered (`webdav_methods_tests`) |
| `mac_gcr_test.cpp` | `lib/media/mac/macGCR.cpp` encode and decode round trip (`mac_gcr_tests`) |
| `AfpServerMatchTests.cpp` | `components/afpfs-ng/lib/server_match.c` address and user matching for server reuse (`afp_server_match_tests`) |
| `AfpGcryptTests.cpp` | `components/afpfs-ng/esp32/` gcrypt shim against RFC 1321, RFC 2144 and DES vectors, DES against MbedTLS, and its random source (`afp_gcrypt_tests`) |
| `AfpMacRomanTests.cpp` | `components/afpfs-ng/lib/macroman.c` MacRoman to UTF-8, including truncation (`afp_macroman_tests`) |
| `AfpUamTests.cpp` | The whole `afpfs_fn` library linked on the host, and its registered login methods (`afp_uam_tests`) |
| `AfpLoopTests.cpp` | The afpfs-ng event loop leaves the host's `SIGINT` and `SIGTERM` handlers in place (`afp_loop_tests`) |
| `AfpSocketTests.cpp` | `components/afpfs-ng/lib/afp_socket.h`: failures reach `errno`, and a write after the peer hung up does not raise `SIGPIPE` (`afp_socket_tests`) |
| `AfpPathTests.cpp` | `lib/afp/afpPath.cpp` splitting a URL path into volume and path (`afp_path_tests`) |
| `AfpSessionTests.cpp` | `lib/afp/afpSession.cpp` sharing of logins and attachments, against the fake in `AfpFakeBridge.h` (`afp_session_tests`) |
| `AfpFileTests.cpp` | `lib/afp/afpFile.cpp` open modes, positions, listings and the directory functions, against the same fake (`afp_file_tests`) |
| `AfpFakeBridge.h` | An in-memory `afpBridge` for the two tests above: one server per user, logout with the last volume, and a small file tree per volume |
| `AfpSessionLiveTests.cpp` | `AfpSession`, `AfpFile` and the directory functions over the real `afpBridge.c` and a real server named by `AFP_TEST_HOST`, `AFP_TEST_USER`, `AFP_TEST_PASSWORD` and `AFP_TEST_VOLUME`, with an optional second account in `AFP_TEST_USER2` and `AFP_TEST_PASSWORD2`; creates and removes `/fujinet-afp-test-<pid>` on that volume; skipped when unset (`afp_session_live_tests`) |
| `AfpServerListTests.cpp`, `AfpServerListHelper.c` | afpfs-ng server reuse by address, signature and user over loopback sockets (`afp_server_list_tests`; not built on Windows) |
| `AfpCloseSessionTests.cpp`, `AfpCloseSessionHelper.c` | afpfs-ng when the server ends a session with a volume mounted: the event loop keeps running and the server is freed later by a caller (`afp_close_session_tests`; not built on Windows) |
| `AfpLoginCauseTests.cpp`, `AfpLoginCauseHelper.c` | The cause `afp_server_full_connect()` leaves in `errno` when a login fails: unresolvable name, refused connection (`afp_login_cause_tests`; not built on Windows) |
| `AfpDirLookupTests.cpp`, `AfpDirLookupHelper.c` | afpfs-ng create, remove, mkdir, rmdir and rename when the parent directory does not exist, against a loopback server on which nothing exists: one lookup, then nothing more is sent (`afp_dir_lookup_tests`; not built on Windows; also links `AfpCloseSessionHelper.c`) |
| `AfpWriteTests.cpp`, `AfpWriteHelper.c` | What `ml_write()` returns when the server refuses a write or a range lock: the errno, or a short count once some bytes are written (`afp_write_tests`; not built on Windows) |
| `CasTapeTests.cpp`, `CassetteRewindRequestTests.cpp`, `CassetteTrailTests.cpp` | Atari cassette record timing, rewind request and trail (`cassette_tests`) |
| `CasFSKTests.cpp` | `lib/media/atari/casFSK.cpp` run scan, chunk checks and timing (`casfsk_tests`) |
| `CasFSKLoaderTests.cpp` | The value-to-symbol encoder in `casFSK.h` and the progressive loader in `lib/media/atari/casFSKLoader.cpp` (`casfskloader_tests`) |
| `CassetteFSKReadTests.cpp` | `lib/device/sio/cassetteFSKRead.cpp` over a fake remote file that loses its position (`cassettefskread_tests`) |
| `FSKTestImages.h` | Synthetic A8CAS images, a fake file and an encoder oracle shared by the two FSK test files above |
| `check_no_build_ifdefs.py` | Policy: no `BUILD_*` macro in a preprocessor conditional under `lib/device/fujiDevice`, `lib/device/fujiClock` or `lib/device/NDevice` (`no_build_ifdefs_in_fujidevice`) |
| `check_no_system_bus.py` | Policy: `SYSTEM_BUS` must not appear in any source under `lib/media` (`no_system_bus_in_media`) |
| `unsit.c`, `stuffit_test.sh` | Host CLI over `lib/stuffit` and a comparison against `unar` (`stuffit_sample_tests`; registered only with `SIT_SAMPLES_DIR`) |
| `ndif_blocks_test.c`, `ndif_test.sh` | NDIF decoding compared with ndif2raw (`ndif_sample_tests`; registered only with `NDIF2RAW_DIR`) |
| `esp_stubs/` | Stand-ins for ESP-IDF headers so ESP-only code compiles on a host: no-op `esp_err.h` and `esp_log.h` for `http_header_tests`, and the `http_method` names in `http_parser.h` for `webdav_methods_tests` |
| `corpus_manifest.txt` | Provenance of the StuffIt sample archives; data only, read by nothing |

## How it fits
- Added by the root `CMakeLists.txt` after `fujinet_pc.cmake`, only when `FUJINET_TARGET` is set.
- doctest comes from [components_pc/doctest/](../components_pc/doctest/); the test file that owns
  `main()` defines `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN`.

## Build
PC only. `make atari-lwm` (or `./build.sh -p ATARI`) configures, builds and runs `ctest -V`; a
failing test aborts the whole invocation. The firmware workflows compile and never run tests.

## Notes
To add a test, add an executable and an `add_test` to `CMakeLists.txt`, and keep the unit under
test free of hardware and FujiNet globals so it links alone. The PlatformIO directory
[test/](../test/) is not this suite.
