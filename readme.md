# C언어를 이용한 정렬 알고리즘 라이브러리

제 개인 학습용 리포지토리입니다. 그럴 일은 없겠지만 원하신다면 마음껏 이용하세요.

윈도우 환경에서만 실행 가능합니다. 싱글스레드 정렬은 해당 함수만을 포함하여 컴파일하면 다른 운영체제에서도 사용 가능합니다.

## 정렬 성능 벤치마크용 exe 파일

`../benchmark/exe` 폴더 내에 GCC로 컴파일 된 벤치마크 exe파일이 있습니다. 안전하지만, 보안 우려가 있다면 소스 코드를 이용하여 직접 빌드 하길 바랍니다.

직접 컴파일하려면, 다음 명령어를 터미널에 실행하면 됩니다.

```bash
gcc -m64 -o <OUTPUT_FILENAME>.exe <BENCHMARK_SOURCE_FILENAME>.c <LIBRARY_SOURCE_FILENAME>.c -O2
```
예를들어, `benchmark_merge_sort.c`를 `merge_sort.c` 라이브러리로 컴파일하고 싶다면 다음 명령어를 터미널에 실해앟면 됩니다. `/benchmark` 폴더 내에서 실행하세요.

```bash
gcc -m64 -o benchmark_merge_sort.exe benchmark_merge_sort.c ../library/merge_sort.c -O2
```

-----------------------------------------------------------------------------

# Sorting Algorithms Library in C

This is a personal repository for educational purposes. It's unlikely to be useful to others, but feel free to use it if you wish.

Current implementation is Windows-only. However, single-threaded sorting functions can be used on other operating systems if compiled separately.

## Benchmark Executables

Pre-compiled benchmark executables (built with GCC) are located in the `../benchmark/exe` directory. 
While they are safe to use, I recommend building from source if you have any security concerns.

To compile the benchmark, run the following command in your terminal:

```bash
gcc -m64 -o <OUTPUT_FILENAME>.exe <BENCHMARK_SOURCE_FILENAME>.c <LIBRARY_SOURCE_FILENAME>.c -O2
```

For example, to compile `benchmark_merge_sort.c` with the `merge_sort.c` library, use the following command. Please execute this from inside the `/benchmark` directory.

```bash
gcc -m64 -o benchmark_merge_sort.exe benchmark_merge_sort.c ../library/merge_sort.c -O2
```