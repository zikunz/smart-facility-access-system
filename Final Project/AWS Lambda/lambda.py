# import the json utility package since we will be working with a JSON object
import json
# import the AWS SDK (for Python the package name is boto3)
import boto3
# import two packages to help us with dates and date formatting
from time import gmtime, strftime
import re

# create a DynamoDB object using the AWS SDK
dynamodb = boto3.resource('dynamodb')
# use the DynamoDB object to select our table
table = dynamodb.Table('EEC_Table')
# store the current time in a human readable format in a variable
now = strftime("%a, %d %b %Y %H:%M:%S +0000", gmtime())

def convert_time(sec):

    days = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    s = int(sec / 1000)
    s -= 60 * 60 * 8       
    m = int((s - s % 60) / 60)
    s %= 60
    h = int((m - m % 60) / 60)
    m %= 60
    dd = int((h - h % 24) / 24)
    h %= 24

    ##12 leap years in total since 1970, algorithm designed for 2022

    yyyy = int((dd - dd % 365) / 365) + 1970
    dd %= 365

    mm = int(dd / 31) + 1

    for i in range(mm-1):
        dd -= days[i]
        i += 1
    dd -= 12;
    res = f"{yyyy:04d}-{mm:02d}-{dd:02d} {h:02d}:{m:02d}:{s:02d}"
    return res

def sortkey(elem):
    return int(elem['time'])
    
# define the handler function that the Lambda service will use as an entry point
def lambda_handler(event, context):
    res = ""
# extract values from the event object we got from the Lambda service and store in a variable

# write name and time to the DynamoDB table using the object we instantiated and save response in a variable
    response = table.scan()
    item = response["Items"]
    item.sort(key = sortkey)
    for elem in item:
        t = convert_time(int(elem['time']))
        res += t
        res += '?'
        userid = elem['USERID']
        uid = str(userid.get('uid'))
        res += uid
        res += '?'
        if uid == "0000":
            res += ""
        if uid == "F21C8B06":
            res += "Ang Li #1"
        elif uid == "04A2D3AA8C5680":
            res += "Ang Li #2"
        else:
            res += "Guest"
        res += '/'

# return a properly formatted JSON object
    return {
        'statusCode': 200,
        'body': str(res)
    }