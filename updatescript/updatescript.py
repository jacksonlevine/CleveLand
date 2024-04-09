from googleapiclient.discovery import build
from google_auth_oauthlib.flow import InstalledAppFlow
from google.auth.transport.requests import Request
from google.oauth2.credentials import Credentials
import os
import io
from googleapiclient.http import MediaIoBaseUpload
import zipfile
from selenium import webdriver
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.common.by import By
import time
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC

with open('../versionstring.txt', 'r') as file:
    version_string = file.read().strip()

html_content = f'<h1>Version</h1><p><br/></p><p>{version_string}</p>'

with open('../ver', 'w') as file:
    file.write(html_content)

def zip_files(zip_filename, files, folders):
    if os.path.exists(zip_filename):
        os.remove(zip_filename)

    with zipfile.ZipFile(zip_filename, 'w') as zipf:
        for file in files:
            zipf.write(file, arcname=os.path.basename(file))

        for folder in folders:
            folder_name = os.path.basename(os.path.normpath(folder))
            for root, _, files in os.walk(folder):
                for file in files:
                    file_path = os.path.join(root, file)
                    arcname = os.path.join(folder_name, os.path.relpath(file_path, folder))
                    zipf.write(file_path, arcname=arcname)

files_to_zip = ['../main.exe', '../ver', '../versionstring.txt']
folders_to_zip = ['../assets']

zip_files('../game.zip', files_to_zip, folders_to_zip)

SCOPES = ['https://www.googleapis.com/auth/drive']
FILE_ID = '1fWHLX7o3UmGbA65J_fL8_rXSeJ3-CKBN'
FILE_PATH = '../game.zip'

creds = None
if os.path.exists('token.json'):
    creds = Credentials.from_authorized_user_file('token.json', SCOPES)
if not creds or not creds.valid:
    if creds and creds.expired and creds.refresh_token:
        creds.refresh(Request())
    else:
        flow = InstalledAppFlow.from_client_secrets_file('credentials.json', SCOPES)
        creds = flow.run_local_server(port=0)
    with open('token.json', 'w') as token:
        token.write(creds.to_json())

service = build('drive', 'v3', credentials=creds)

file_metadata = {'name': 'game.zip'}
media = MediaIoBaseUpload(io.BytesIO(open(FILE_PATH, 'rb').read()), mimetype='application/zip')
updated_file = service.files().update(fileId=FILE_ID, body=file_metadata, media_body=media).execute()
print('Uploaded new game.zip version:', updated_file['id'])

with open('tumblr_email.txt', 'r') as file:
    tumblr_email = file.read().strip()
with open('tumblr_password.txt', 'r') as file:
    tumblr_password = file.read().strip()

post_id = '738667692102467584'
new_content = 'Your new post content'

webdriver_path = '../../chromedriver.exe'

service = Service(webdriver_path)
driver = webdriver.Chrome(service=service)

driver.get('https://www.tumblr.com/login')
time.sleep(2)  # Wait for the page to load

email_input = WebDriverWait(driver, 20).until(
    EC.presence_of_element_located((By.NAME, "email"))
)
email_input.send_keys(tumblr_email)

password_input = WebDriverWait(driver, 20).until(
    EC.presence_of_element_located((By.NAME, "password"))
)
password_input.send_keys(tumblr_password + Keys.RETURN)

WebDriverWait(driver, 20).until(
    EC.url_contains("dashboard")
)

edit_url = 'https://www.tumblr.com/edit/clevelandnews/738667692102467584'
driver.get(edit_url)
time.sleep(2)

wait = WebDriverWait(driver, 30)
editor = wait.until(EC.presence_of_element_located((By.XPATH, '/html/body/div[1]/div/div/div[4]/div/div/div/div/div/div/div[2]/div/div[1]/div[2]/div/div[3]/div[2]/div/div/div[1]/p[2]')))

editor.click()

editor.send_keys(Keys.CONTROL, 'a')

editor.send_keys(Keys.DELETE)

new_post_content = version_string
editor.send_keys(new_post_content)

save_button = wait.until(EC.element_to_be_clickable((By.XPATH, '/html/body/div[1]/div/div/div[4]/div/div/div/div/div/div/div[2]/div/div[3]/div/div/div/button/span')))
save_button.click()

post_button = wait.until(EC.element_to_be_clickable((By.XPATH, '/html/body/div[1]/div/div/div[4]/div[2]/div[2]/div/div[2]/button[1]/span')))
post_button.click()
time.sleep(2)

driver.quit()