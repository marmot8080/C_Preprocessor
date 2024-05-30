# C_Preprocessor
C언어에서 #include와 #define의 전처리를 실행하는 preprocessor

# 실행파일
실행하고자 하는 파일의 절대경로를 포함한 파일명 입력시 전처리 실행
* .cpp, .c 파일 이외의 파일은 실행에 오류가 발생할 수 있음

# 참고
- Visual Studio 2022를 이용해 작성한 코드이기 때문에 다른 컴파일러를 사용할 경우 _s() 함수부분 수정 필요 (ex. scanf_s(), strtok_s(), strcpy_s() 등)
- #define은 정의한 변수의 이름이 같은 다른 매크로가 정의되어 있을 경우, 나중에 정의된 #define의 상수값으로 전처리 진행
