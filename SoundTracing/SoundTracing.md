# [SoundTracing](sound_tracing논문_appendix포함.pdf)

## Sound Rendering System

## Sequence

- Setup Processing
- Ray Generation
- T&I ( Treversal & Intersection )
- GPS ( Guide Plane Sort )
- RGCnRPS ( Reverb Geometry Collection & Reverb Plane Sort )
- PPV ( Propagation Path Validation )

### Setup Processing

---

### Ray Generation

---

목적 : 공간 Sampling을 위한 Ray Generation

- Ray Generation은 공간 Sampling( Plane 수집 )을 위해 GuideRay와 SourceRay를 Omni Directional으로 생성한다 GuidRay의 경우 1024개의 최대 Ray 개수를 가지며, SourceRay의 경우 128개의 최대 Ray 개수를 가진다.

- 생성된 Ray는 T&I Unit으로 전달돼 처리된다.

### T & I

---

목적 : Triangle Sequence 수집 ( Sampling 수행 )

T&I Unit은 Ray Generation 에서 생성된 Ray에 대해서  depth(최대 4) 만큼 Traversal 과 Intersection을 수행한다. 이 과정에서 사용자에게 도달할 수 있는 Traversal 과정에서 충돌된 Triangle 수집한다.

- **KDTree** 가속화 구조를 사용하여 성능을 향상시킴
- KDTree는 공간을 K 차원으로 분할하여 Tree를 생성하는 방식

- 스택구조를 사용하지 않는다. ( RayCore의 경우 Stack이 필요하다 [ 하나의 Hit Point에서 여러개의 2차 Ray가 생성되기 때문 { Shadow, Reflection, refraction } ] ),
  하지만 Sound Tracing 관점에서는 하나의 Hit Point에서는 하나의 Reflection 혹은 Diffraction Ray만 생성되기 때문이다.

- 단일 파이프라인을 사용하여 필요 chip 공간을 감소시켰다. ( 어째서 단일 파이프라인? => 실시간 Sound Tracing처리에 단일 파이프라인으로 충분하기 때문 )

### GPS

---

목적 : Triangle을 Coplane으로 병합하여 Plane Sequence 생성

GPS (Guide Plane Sort) Unit의 경우, GuideRay를 T&I하여 수집된 모든 Triangle Sequence에 대하여 Coplane으로 병합하는 작업을 Depth 마다 반복적으로 수행한다, Merging process 하드웨어로 가속된 Merge Sorting [Nah et al. 2014] 하여 Plane Sequence을 생성한다

- Plane Sequence란 특정 Plane에 대해서 Ray의 Reflection이 발생한 경우에 가능한 전파 경로의 Caching을 의미한다. ( 어떤 데이터 구조를 사용하고 있는가 ? [ Tree, Map, etc.. ] )
- 이를 통해 Reflection과 Late Reverberation을 계산한다.

- 이를 통해 음원의 개수가 늘어나도 각 음원 별로 Ray tracing을 수행하는 것이 아닌 Listener의 Transform을 통해 계산된 Plane Sequence를 이용해 계산하기 때문에 음원으 개수가 증가해도 Reflection 계산에 영향을 주지 않는다.
  그러나 late Reverberation은 SoundSource 기준에서 SourceRay를 통해 Sampling하기 때문에 이에 대해서는 계산량이 증가한다.
- Merge Sort
  - 기준
    - Depth
    - Normal
    - Offset
      - Plane간의 거리
      - Reference Point간의 거리
- 특징
  - 하드웨어 Merge Sorting operation 방식을 통해 성능 향상 [Nah et al. 2014]
  - Depth 4까지 재귀적으로 수행
  - 생성된 Sequence는 Dedicated Plane Buffer에 저장

- Coplane
  - 기준
    - Tolerance [0.999 and 0.001]
    - Triangle에 대하여
      - Same normal
      - Same offset

- Plane Buffer
  - Count : 16 ( 16개의 sound source 지원 )
  - Size : ( 8 frame ) *( 1024 guide rays + 128 source rays )* ( 4 depth )

### RGCnRPS

---

목적

- GPS와 그 목적이 동일하다. 단, Source Ray를 타겟으로 작동한다.

RGCnRPS ( Reverb Geometry Collection & Reverb Plane Sort )

- RGC : GPS와 동일한 방식으로 Source Ray에 대하여 병합 작업을 수행하여 Plane Sequence 생성성

- Late Reverberation을 계산

- Eyring’s model [Eyring 1930]을 활용함

### PPV

---

PPV

목적 : Plane Sequence의 Path들에 대해서 Validate를 수행행

- Direct path occlusion test
- Reflection path validation with **Image Source Method**
- Diffraction path validation with **Shadow Region** and **Edge Detection**
  
설명명

- Direct path occlusion test
  - Source to Listener Ray에 대해서 Occlusion 검사 수행
    - Ray Intersection

- Reflection path validation
  - Image Source Method 방식을 통해 Validation 수행
    1. Plane Sequence의 Plane에 대한 Listener Position의  Mirror Position 생성
    2. Sound Source에서 Listener까지 Occlusion Test 수행
    3. Image Source Method를 통해 Room Impulse Response를 계산

- Diffraction path validation
  - Shadow region에 위치한 SoundSource에 대해서 Edge Detection을 수행하여 Diffraction path 생성
    - Guide Ray의 Hit Point에서 Triangle의 각 모서리에 대해서 각 Triangle의 Normal에 수직인 Tangent 방향에 Epsilon크기를 가지는 Vector를 edge point에 더하거나 빼는 방식으로 Sub-edge point를 생성한다.
      - Sub-edge point = edge point +- epsilon ( 0.001 )

    - 검사하는 3가지 Ray
      - 1. Lister to Triangle Ray
      - 2. Lister to Sub-edge point Ray
      - 3. Soundsource to sub-edge point Ray

    - 1번의 Ray만 충돌하고, 2,3번의 Ray는 충돌하지 않았을때, Diffraction의 가능성이 있다고 판정한다.
      - edge detection 방식은 **approximation method**이기 때문에, 다른 전처리 method에서 발견한 diffraction을 발견하지 못할 수 있다. 대신  dynamic scenes을 지원하는 것이 가능하다.

    - 1차 회절까지 계산

- GSound와 비교 했을때의 강점
  - 이는 Plane base method와 Triangle base method의 비교라고 볼 수 있다. Plane base method의 경우 valid path의 탐색의 가능성이 더 높다. 그 이유는 triangle base method의 경우 동일한 평면상의 Triangle이지만 GuideRay에 탐지되지 않았을 경우 Validation이 실패하기 때문이다. 이러한 문제점은 Surface가 거대할때 더 빈번하게 발생한다.
  - Triangle base method의 경우도 GuideRay의 개수를 증가시킬 경우 이러한 문제점을 해결할 수 있지만, 계산량 또한 증가하게 된다.
  - 탐색 가능성이 높다는 것은 더 적은 T&I 계산으로 더 정확한 Propagation을 수행할 수 있다는 의미이다.

### Path Cache Buffer & IRCalculator

---

#### Path Cache Buffer

- Valid Path가 적재되어 있는 Buffer
- 렌더링 되는 프레임의 일관성을 유지하기 위해 사용한다. ( 일관성이 유지 되지 않으면, 소리가 갑자기 달라지는 등의 문제가 발생할 것으로 예상 )
- 이전 프레임에 제공되거나 업데이트된 Valid Paths를 통해 Valid Paths의 해상도를 향상시킨다.
- Sound Source 마다 32개의 Valid Paths를 Path Cache Buffer에 저장할 수 있다.
- 데이터 구조
  - Triangle
  - Depth
  - Primitive Index
  - Normal
  - Offset Per Depth

`Question : Path Cache Buffer가 어떠한 방식으로 Frame Coherency problem을 해결하고 있는가 ?`

#### IRCalculator

- Listener와 SoundSource간의 Impulse Response를 계산하는 Unit
- 4 frequency bands( 0~250Hz, 250Hz~1000Hz, 1000Hz~4000Hz, and >4000Hz )를 지원한다
- 2개의 Path 입력을 가진다.
  - 1. Valid Path Buffer
    - Delay 계산 ( Total path length / sound speed(343m/s))
    - [Attenuation Parameter 계산]()
      - 거리 + 누적된 충돌 표면 흡수 계수 + [UTD 모델 ( Diffraction )](./Diffraction/Modeling%20Acoustics%20in%20Virtual%20Environments%20Using%20the%20Uniform%20Theory%20of%20Diffraction.pdf)
  - 2. RGCnRPS Path Data
    - [Eyring Model](./Reverberation/CF%20Eyring-Reverb%20Time%20in%20Dead%20Rooms.pdf)의 계산에 필요한 Parameters 계산
      - Total area of room (totalArea)
      - Total absorbing surface area (freqAtten)
      - Average absorption coefficient of the surfaces (reverbAvgAtten)
    - 계산된 결과는 IRBuffer에 저장되어 외부 메모리로 전달된다. 외부 시스템에서는 해당 메모리를 읽어 최종 Sound Effect를 생성한다.

### Sound Tracing의 강점

- Hardware-based Sound Rendering System
- T&I
  - 단일 Pipeline 사용으로 Chip area 축소
  - SPU를 통한 Batch Processing
  - KD-Tree 가속화 구조를 활용한 성능 향상 ( non-blocking cache system , unified MIMD 기능을 사용할 수 있게함 )
- PPV
  - Plane Base method : Triangle Base method에 비해 적은 Sampling으로 더 정확한 Path validation이 가능

## To Learn

- [Image Source Method :  Jont B Allen and David A Berkley. 1979. Image method for efficiently simulating small-room acoustics. The Journal of the Acoustical Society of America 65, 4 1979, 943–950](Multi-Threaded%20Sound%20Propagation%20Algorithm%20to%20Improve%20Performance%20on%20Mobile%20Devices.pdf)
  - [MathWorks image source method example](https://kr.mathworks.com/help/audio/ug/room-impulse-response-simulation-with-image-source-method-and-hrtf-interpolation.html)
- Hardware Merge Sorting Operation :  [Nah et al. 2014] 란
- Reverb Model : Eyring’s model [Eyring 1930]란
- KD-Tree 란?
- Non-blocking Caches

## what?

- unified MIMD

## Keyword

- GA-based sound propagation
- GSound
- RayCore
- Batch
- SPU
- Chip Area
- KD-Tree


## Research Keword

- wave-based numerical method ( Chaitanya et al. 2020; Liu and Liu 2020; Mehra et al. 2013, 2015; Raghuvanshi et al. 2009; Raghuvanshi and Snyder 2018; Raghuvanshi et al. 2010)
- geometric acoustic (GA) method 
  - Schissler and Manocha 2016; Schissler et al. 2014; Taylor et al. 2009a, 2011, 2012
  - Lauterbach et al. 2007; Tsingos et al. 2001
- hybrid method ( [Southern et al. 2013; Tang et al. 2021; Yeh et al. 2013] )
- GA based Sound diffuse calculation 

- static approach
  - Taylor et al. [2009a] proposed a hybrid ray-tracing
technique that utilizes both discrete rays and frusta. This method
also employs a statistical approach to calculate the reverberation
IR and combines the outcomes from these two types of sound simu-
lations

- dynamic approach
  - In a subsequent study by Taylor et al. [2012], early specular
reflection and first-order diffraction were accelerated through a
multi-view tracing algorithm implemented on a GPU

- gsound
  - Schissler et al. [2011] presented GSound, an efficient CPU-based
approach that builds upon the previous GPU-based method called
iSound [Taylor et al. 2010]

- diffuse
  - Schissler et al. [2014] proposed an iterative method that combines
radiosity and path tracing to calculate diffuse reflection

- real-time with multiple sound source
  - Addition-
ally, Schissler et al. [2016] introduced a real-time approach capable
of processing higher-order paths and multiple sound sources lo-
cated far from the listener

- dynamic diffraction algorithm and preprocessing to find diffraction edges
  - More recently, [2021]
presented a new dynamic diffraction algorithm that combines mesh
preprocessing to find diffraction edges and stochastic bidirectional
path tracing [Cao et al. 2016]

- preprocessing-base method
  - reduce the computational cost of ray traversal and intersection test
    - One of the frequently adopted algorithms is preprocessed geometry simplification [Schissler et al. 2014, 2021; Siltanen et al. 2008] 
  - improves the accuracy for GA by increasing the size of planer surfaces relative to the wavelength
    - [Savioja and Svensson 2015].
  - preprocessing dege detection
    -  Taylor et el.
[2009b] and Schissler et al. [2011] identify edge candidates based
on angles between adjacent triangles, and Schisseler et al. [2014;
2021] accelerate the process by creating a visibility graph to support
high-order diffraction generated through multiple edges.

