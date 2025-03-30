import os
import requests
from bs4 import BeautifulSoup

################## 사용자의 수정이 필요한 부분 ##################
###############################################################
# 웹페이지 URL
# 특정 탭의 콘텐츠 ID를 변수로 정의 
# 추출할 아이템 클래스 이름을 리스트로 정의

#url = "https://poedb.tw/kr/Divination_Cards"
#target_id = "점술카드아이템"
#class_names = ["divination"]

# url = "https://poedb.tw/kr/Pieces"
# target_id = "조각아이템"
# class_names = ["whiteitem"]

# url = "https://poedb.tw/kr/Incubators"
# target_id = "인큐베이터아이템"
# class_names = ["whiteitem"]

# url = "https://poedb.tw/kr/Corpses"
# target_id = "시신아이템"
# class_names = ["whiteitem"]

# url = "https://poedb.tw/kr/Embers_of_the_Allflame"
# target_id = "시신아이템"
# class_names = ["whiteitem"]

# url = "https://poedb.tw/kr/Maps#%EC%A7%80%EB%8F%84%EA%B3%A0%EC%9C%A0"
# target_id = "지도고유"
# class_names = ["item_unique"]

# url = "https://poedb.tw/kr/Maps#%EC%A7%80%EB%8F%84%EC%95%84%EC%9D%B4%ED%85%9C"
# target_id = "지도아이템"
# class_names = ["itemclass_map map_tier1", "itemclass_map map_tier2", "itemclass_map map_tier3", "itemclass_map map_notinatlas"]

# url = "https://poedb.tw/kr/Maps#%EC%A7%80%EB%8F%84%EC%95%84%EC%9D%B4%ED%85%9C"
# target_id = "기타지도아이템아이템"
# class_names = ["whiteitem"]

# url = "https://poedb.tw/kr/Scarab"
# target_id = "갑충석아이템"
# class_names = ["whiteitem"]

# url = "https://poedb.tw/kr/Breachstone"
# target_id = "균열석아이템"
# class_names = ["whiteitem"]

# url = "https://poedb.tw/kr/Emblem"
# target_id = "Emblem아이템" #상징
# class_names = ["whiteitem"]

# url = "https://poedb.tw/kr/Skill_Gems#%EC%8A%A4%ED%82%AC%EC%A0%AC%EC%A0%AC"
# target_id = "스킬젬젬"
# class_names = ["gem_red", "gem_green", "gem_blue", "gemitem"]

# url = "https://poedb.tw/kr/Stackable_Currency"
# target_id = "중첩가능화폐아이템"
# class_names = ["item_currency"]

# url = "https://poedb.tw/kr/Stackable_Currency"
# target_id = "에센스"
# class_names = ["item_currency"]

# url = "https://poedb.tw/kr/Delirium_Orb"
# target_id = "환영의오브아이템"
# class_names = ["item_currency"]

# url = "https://poedb.tw/kr/Stackable_Currency"
# target_id = "Vial아이템"
# class_names = ["item_currency"]

# url = "https://poedb.tw/kr/Stackable_Currency"
# target_id = "Splinter아이템"
# class_names = ["item_currency"]

# url = "https://poedb.tw/kr/Stackable_Currency"
# target_id = "성유아이템"
# class_names = ["item_currency"]

# url = "https://poedb.tw/kr/Stackable_Currency"
# target_id = "화석아이템"
# class_names = ["item_currency"]

# url = "https://poedb.tw/kr/Stackable_Currency"
# target_id = "Catalyst아이템"
# class_names = ["item_currency"]

url = "https://poedb.tw/kr/Tattoo"
target_id = "문신아이템"
class_names = ["item_currency"]
###############################################################

# 웹페이지 HTML 가져오기
response = requests.get(url)
html = response.text

# BeautifulSoup으로 HTML 파싱
soup = BeautifulSoup(html, 'html.parser')

# 특정 탭의 콘텐츠만 선택
target_section = soup.find("div", {"id": target_id})

# 로그 추가: 특정 탭의 콘텐츠가 선택되었는지 여부 확인
if target_section:
    print(f"'{target_id}' 섹션이 성공적으로 선택되었습니다.")
else:
    print(f"'{target_id}' 섹션을 찾을 수 없습니다. HTML 구조를 확인하세요.")

# 특정 클래스들의 `a` 태그의 텍스트만 추출 (지정된 섹션 내에서만 검색)
blue_keywords = []
if target_section:
    for class_name in class_names:
        for item in target_section.find_all("a", class_=class_name):
            text = item.text.strip()  # 앞뒤 공백 제거
            if text:  # 빈 문자열이 아닌 경우만 추가
                blue_keywords.append(text)

# 결과 출력
print(f"'{target_id}' 아이템 제목:", blue_keywords)
print(f"총 아이템 개수: {len(blue_keywords)} 개")

# 자음 추출 함수 정의 (한글 초성만 추출, 숫자는 '?'로 대체)
def extract_consonants(hangul_text):
    CHO_SEONG_LIST = [
        'ㄱ', 'ㄲ', 'ㄴ', 'ㄷ', 'ㄸ', 'ㄹ', 'ㅁ', 'ㅂ', 'ㅃ', 'ㅅ', 'ㅆ', 'ㅇ', 'ㅈ', 'ㅉ', 'ㅊ', 'ㅋ', 'ㅌ', 'ㅍ', 'ㅎ'
    ]
    consonants = []
    for char in hangul_text:
        if '가' <= char <= '힣':  # 한글 음절인지 확인
            char_code = ord(char) - ord('가')
            initial_code = char_code // 588
            consonant = CHO_SEONG_LIST[initial_code]
            consonants.append(consonant)
        elif 'ㄱ' <= char <= 'ㅎ':  # 이미 분리된 자음일 경우
            consonants.append(char)
        elif '0' <= char <= '9':  # 숫자인 경우 '?'로 대체
            consonants.append('??')
    return ''.join(consonants)

# 자음만 추출하여 리스트에 저장
consonant_keywords = [extract_consonants(keyword) for keyword in blue_keywords]

# 현재 스크립트 파일 경로를 기준으로 텍스트 파일 저장 경로 설정
base_dir = os.path.dirname(os.path.abspath(__file__))  # 현재 스크립트 파일의 디렉토리
output_dir = os.path.join(base_dir, "Contents", "TextFile")

# 디렉토리가 없을 경우 생성
os.makedirs(output_dir, exist_ok=True)

# 파일 저장 경로 설정 (target_id를 파일 이름으로 사용)
output_file_path = os.path.join(output_dir, f"{target_id}.txt")

# 텍스트 파일로 저장 (C++ 코드 형태로 저장, 5개마다 주석 추가하고 아닌 경우 한 줄 띄우기, 마지막 데이터에 주석 추가)
with open(output_file_path, "w", encoding="utf-8") as file:
    for index, (consonant, original) in enumerate(zip(consonant_keywords, blue_keywords), start=1):
        file.write(f'            database.emplace_back("{consonant}", "{original}");')
        if index % 5 == 0 or index == len(blue_keywords):  # 5개마다 주석 추가 또는 마지막 데이터에 주석 추가
            file.write(f' // {index}\n\n')
        else:  # 5개마다 주석이 아닐 때 빈 줄 추가
            file.write('\n')

print(f"데이터가 '{output_file_path}' 파일로 저장되었습니다.")
