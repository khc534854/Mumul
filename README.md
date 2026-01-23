# Metaverse Platform Project

## 1. 개요 (Overview)

멀티플레이 기반 메타버스 플랫폼으로,  
실시간 네트워크 통신, 플레이어 상호작용, 데이터 기반 콘텐츠 확장성을 핵심 목표로 설계·구현한 프로젝트입니다.

UGameInstanceSubsystem을 중심으로 HTTP / WebSocket 통합 네트워크 구조를 설계했으며,  
컴포넌트 기반 플레이어 콘텐츠(하우징, 커스터마이징, 회의 시스템)를 통해  
확장성과 협업 효율을 고려한 아키텍처를 구축했습니다.

### 기술 스택
- Engine: Unreal Engine 5
- Language: C++
- Network: HTTP, WebSocket
- Architecture: UGameInstanceSubsystem, Component-based Design
- Data: DataTable, DataAsset, JSON Serialization
- Collaboration Tools: Git, Jira, Figma / FigJam
- ETC: Steam Online Subsystem, PCG Plugin

---

## 2. 담당 역할 (Role)

- 협업 툴 세팅 및 일정 관리, 회의 진행
- 언리얼 개발 일정 리딩 및 업무 분담
- 멀티플레이 및 네트워크 통신 핵심 기능 구현

---

## 3. 핵심 기능 및 시스템 (Key Features & Systems)

### 3.1 Subsystem 기반 네트워크 아키텍처

- UGameInstanceSubsystem 기반 전역 네트워크 시스템 설계
- HTTP / WebSocket 하이브리드 통신 구조
  - HTTP: 유저 인증, 데이터 로드·세이브
  - WebSocket: 실시간 메시지 및 AI 에이전트 연동
- 비동기 통신 처리로 메인 스레드 블로킹 방지

#### 리플렉션 기반 API 자동화
- C++ 템플릿 + StaticStruct() + FJsonObjectConverter 활용
- 구조체 정의만으로 JSON 직렬화/역직렬화 자동 처리
- 문자열 기반 파싱 제거 및 타입 안정성 확보
- API 요청 코드 약 65% 이상 감소

---

### 3.2 메타버스 플레이어 핵심 콘텐츠

#### 커스터마이징 시스템
- Data-Driven 설계
  - FCustomItemData 구조체 + DataAsset 연동
- UI와 캐릭터 에셋 간 동적 데이터 바인딩
- 신규 아이템 추가 시 코드 수정 없이 확장 가능

#### 하우징 시스템
- Preview System
  - Ghost Actor를 활용한 배치 전 미리보기 및 충돌 검사
- Placement Logic
  - 라인 트레이싱 기반 지형 인식
  - 그리드 / 자유 배치 모드 지원

#### 회의(상호작용) 시스템
- PlayerState 기반 참여 상태 동기화
- 회의 시작·종료는 서버 권위(Server Authority)로 제어
- 음성 데이터 처리
  - 오디오를 1분 단위 WAV Raw 데이터로 분할
  - HTTP 멀티파트 폼데이터 방식으로 서버 전송
  - AI 요약 시스템 연동을 고려한 구조

---

### 3.3 보이스 챗 시스템

- 거리 기반 음성 채팅 (Spatial Voice Chat)
  - Sound Attenuation을 활용한 거리 감쇠 및 3D 음향
- 그룹 채널 효과
  - 특정 구역 진입 시 Attenuation 파라미터 실시간 변경
- 별도 서버 채널 분리 없이 엔진 오디오 시스템만으로 공간 채널링 구현

---

### 3.4 PCG 기반 레벨 디자인

- Unreal PCG Plugin 활용
- Difference Node를 이용해 설치 가능 / 불가능 영역 분리
- 환경 오브젝트 자동 배치
  - 숲, 초원: 나무, 돌, 풀, 꽃
  - 해안가: 야자수, 모래사장 오브젝트
- 게임 실행 시 런타임 Generate 방식
- 플레이어 설치 오브젝트(텐트)에 태그 부여
  - PCG Difference Node로 생성 영역에서 제외

#### PCG 활용 이유 및 장점
- 넓은 맵에서 수작업 배치 비용 절감
- 플레이어 설치물과 환경 오브젝트 간 충돌 방지
- 재접속 시에도 자연스러운 월드 상태 유지
- 월드 확장 및 규칙 변경 시 재생성 용이

---

## 4. 결과 및 회고 (Result & Retrospective)

- 결과 : 기업
