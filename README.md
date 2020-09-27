# genome_matching

2018.05 ~ 2018.06

DNA패턴매칭

개발 및 실행환경

Windows 8.1

Intel Core i-5 5200U CPU 2.20GHz   8GB RAM

visual studio 2017

C++

사용알고리즘: Boyer-Moore

*Bad character
문자가 불일치하면 테이블에서 해당문자를 찾아 점프 (평균 테이블 크기의 절반만큼 점프)

*Good suffix
case1: 접미부가 앞에 존재하면 그만큼 점프
case2: 접미부가 앞에 존재하지 않으면 접미부의 일부분을 보고 접두부와 일치하면 그만큼 점프(접미부와 접두부 사이크기만큼 점프가능)

벤치마크: KMP

prefix - suffix를 이용하여 suffix에서 불일치 할 경우 prefix로 점프해서 비교.




테스트
Reference 길이 : 10000
mismatch : 100
shortread : 2000개

