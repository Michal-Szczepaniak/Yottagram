package text

import (
	"sort"
	"strings"
)

// Slack's emoji names, for the common ones. Slack has no API for the standard
// set, and custom emoji have no Unicode form, so anything else stays a name.
var emojiNames = map[string]string{
	"grinning": "😀", "smiley": "😃", "smile": "😄", "grin": "😁", "laughing": "😆", "satisfied": "😆",
	"sweat_smile": "😅", "rolling_on_the_floor_laughing": "🤣", "rofl": "🤣", "joy": "😂",
	"slightly_smiling_face": "🙂", "upside_down_face": "🙃", "wink": "😉", "blush": "😊", "innocent": "😇",
	"smiling_face_with_3_hearts": "🥰", "heart_eyes": "😍", "star-struck": "🤩", "kissing_heart": "😘",
	"kissing": "😗", "relaxed": "☺", "kissing_closed_eyes": "😚", "kissing_smiling_eyes": "😙", "yum": "😋",
	"stuck_out_tongue": "😛", "stuck_out_tongue_winking_eye": "😜", "zany_face": "🤪",
	"stuck_out_tongue_closed_eyes": "😝", "money_mouth_face": "🤑", "hugging_face": "🤗", "hugs": "🤗",
	"face_with_hand_over_mouth": "🤭", "shushing_face": "🤫", "thinking_face": "🤔", "thinking": "🤔",
	"zipper_mouth_face": "🤐", "face_with_raised_eyebrow": "🤨", "neutral_face": "😐", "expressionless": "😑",
	"no_mouth": "😶", "smirk": "😏", "unamused": "😒", "face_with_rolling_eyes": "🙄", "roll_eyes": "🙄",
	"grimacing": "😬", "lying_face": "🤥", "relieved": "😌", "pensive": "😔", "sleepy": "😪",
	"drooling_face": "🤤", "sleeping": "😴", "mask": "😷", "face_with_thermometer": "🤒",
	"face_with_head_bandage": "🤕", "nauseated_face": "🤢", "face_vomiting": "🤮", "sneezing_face": "🤧",
	"hot_face": "🥵", "cold_face": "🥶", "woozy_face": "🥴", "dizzy_face": "😵", "exploding_head": "🤯",
	"face_with_cowboy_hat": "🤠", "partying_face": "🥳", "sunglasses": "😎", "nerd_face": "🤓",
	"face_with_monocle": "🧐", "confused": "😕", "worried": "😟", "slightly_frowning_face": "🙁",
	"white_frowning_face": "☹", "open_mouth": "😮", "hushed": "😯", "astonished": "😲", "flushed": "😳",
	"pleading_face": "🥺", "frowning": "😦", "anguished": "😧", "fearful": "😨", "cold_sweat": "😰",
	"disappointed_relieved": "😥", "cry": "😢", "sob": "😭", "scream": "😱", "confounded": "😖",
	"persevere": "😣", "disappointed": "😞", "sweat": "😓", "weary": "😩", "tired_face": "😫", "yawning_face": "🥱",
	"triumph": "😤", "rage": "😡", "angry": "😠", "face_with_symbols_on_mouth": "🤬", "smiling_imp": "😈",
	"imp": "👿", "skull": "💀", "skull_and_crossbones": "☠", "hankey": "💩", "poop": "💩", "shit": "💩",
	"clown_face": "🤡", "japanese_ogre": "👹", "japanese_goblin": "👺", "ghost": "👻", "alien": "👽",
	"space_invader": "👾", "robot_face": "🤖", "smiley_cat": "😺", "smile_cat": "😸", "joy_cat": "😹",
	"heart_eyes_cat": "😻", "see_no_evil": "🙈", "hear_no_evil": "🙉", "speak_no_evil": "🙊",

	"wave": "👋", "raised_back_of_hand": "🤚", "hand": "✋", "raised_hand": "✋", "spock-hand": "🖖",
	"ok_hand": "👌", "pinching_hand": "🤏", "v": "✌", "crossed_fingers": "🤞", "the_horns": "🤘",
	"metal": "🤘", "call_me_hand": "🤙", "point_left": "👈", "point_right": "👉", "point_up_2": "👆",
	"middle_finger": "🖕", "point_down": "👇", "point_up": "☝", "+1": "👍", "thumbsup": "👍", "-1": "👎",
	"thumbsdown": "👎", "fist": "✊", "facepunch": "👊", "punch": "👊", "left-facing_fist": "🤛",
	"right-facing_fist": "🤜", "clap": "👏", "raised_hands": "🙌", "open_hands": "👐", "palms_up_together": "🤲",
	"handshake": "🤝", "pray": "🙏", "writing_hand": "✍", "nail_care": "💅", "muscle": "💪", "eyes": "👀",
	"eye": "👁", "brain": "🧠", "tongue": "👅", "lips": "👄", "shrug": "🤷", "facepalm": "🤦", "bow": "🙇",

	"heart": "❤", "orange_heart": "🧡", "yellow_heart": "💛", "green_heart": "💚", "blue_heart": "💙",
	"purple_heart": "💜", "black_heart": "🖤", "white_heart": "🤍", "brown_heart": "🤎", "broken_heart": "💔",
	"two_hearts": "💕", "sparkling_heart": "💖", "cupid": "💘", "gift_heart": "💝", "heartpulse": "💗",
	"heartbeat": "💓", "revolving_hearts": "💞", "kiss": "💋", "100": "💯", "anger": "💢", "boom": "💥",
	"collision": "💥", "dizzy": "💫", "sweat_drops": "💦", "dash": "💨", "bomb": "💣", "speech_balloon": "💬",
	"thought_balloon": "💭", "zzz": "💤",

	"white_check_mark": "✅", "heavy_check_mark": "✔", "ballot_box_with_check": "☑", "x": "❌",
	"negative_squared_cross_mark": "❎", "heavy_plus_sign": "➕", "heavy_minus_sign": "➖", "warning": "⚠",
	"no_entry": "⛔", "no_entry_sign": "🚫", "question": "❓", "grey_question": "❔", "exclamation": "❗",
	"heavy_exclamation_mark": "❗", "grey_exclamation": "❕", "bangbang": "‼", "interrobang": "⁉",
	"sparkles": "✨", "star": "⭐", "star2": "🌟", "zap": "⚡", "fire": "🔥", "snowflake": "❄",
	"rainbow": "🌈", "sunny": "☀", "cloud": "☁", "umbrella": "☔", "droplet": "💧", "ocean": "🌊",
	"tada": "🎉", "confetti_ball": "🎊", "balloon": "🎈", "gift": "🎁", "trophy": "🏆", "medal": "🏅",
	"first_place_medal": "🥇", "second_place_medal": "🥈", "third_place_medal": "🥉", "dart": "🎯",
	"bulb": "💡", "mag": "🔍", "lock": "🔒", "unlock": "🔓", "key": "🔑", "bell": "🔔", "no_bell": "🔕",
	"loudspeaker": "📢", "mega": "📣", "pushpin": "📌", "paperclip": "📎", "link": "🔗", "wrench": "🔧",
	"hammer": "🔨", "hammer_and_wrench": "🛠", "gear": "⚙", "package": "📦", "memo": "📝", "pencil": "📝",
	"pencil2": "✏", "book": "📖", "books": "📚", "calendar": "📆", "date": "📅", "clock": "🕐",
	"hourglass": "⌛", "stopwatch": "⏱", "alarm_clock": "⏰", "email": "📧", "envelope": "✉", "phone": "☎",
	"iphone": "📱", "computer": "💻", "desktop_computer": "🖥", "keyboard": "⌨", "camera": "📷", "tv": "📺",
	"musical_note": "🎵", "notes": "🎶", "headphones": "🎧", "microphone": "🎤", "video_game": "🎮",
	"chart_with_upwards_trend": "📈", "chart_with_downwards_trend": "📉", "bar_chart": "📊",
	"moneybag": "💰", "dollar": "💵", "credit_card": "💳", "gem": "💎", "rocket": "🚀", "airplane": "✈",
	"car": "🚗", "bike": "🚲", "ship": "🚢", "construction": "🚧", "rotating_light": "🚨", "house": "🏠",
	"office": "🏢", "earth_africa": "🌍", "earth_americas": "🌎", "earth_asia": "🌏", "crown": "👑",
	"eyeglasses": "👓", "tophat": "🎩", "dog": "🐶", "cat": "🐱", "mouse": "🐭", "rabbit": "🐰", "fox_face": "🦊",
	"bear": "🐻", "panda_face": "🐼", "tiger": "🐯", "lion_face": "🦁", "cow": "🐮", "pig": "🐷", "frog": "🐸",
	"monkey_face": "🐵", "chicken": "🐔", "penguin": "🐧", "bird": "🐦", "duck": "🦆", "owl": "🦉", "bee": "🐝",
	"bug": "🐛", "butterfly": "🦋", "snail": "🐌", "turtle": "🐢", "snake": "🐍", "fish": "🐟", "whale": "🐳",
	"unicorn_face": "🦄", "seedling": "🌱", "evergreen_tree": "🌲", "deciduous_tree": "🌳", "cactus": "🌵",
	"herb": "🌿", "four_leaf_clover": "🍀", "maple_leaf": "🍁", "rose": "🌹", "sunflower": "🌻", "tulip": "🌷",
	"apple": "🍎", "banana": "🍌", "grapes": "🍇", "watermelon": "🍉", "lemon": "🍋", "strawberry": "🍓",
	"peach": "🍑", "avocado": "🥑", "hamburger": "🍔", "fries": "🍟", "pizza": "🍕", "taco": "🌮", "burrito": "🌯",
	"sushi": "🍣", "ramen": "🍜", "cookie": "🍪", "cake": "🍰", "birthday": "🎂", "doughnut": "🍩",
	"popcorn": "🍿", "coffee": "☕", "tea": "🍵", "beer": "🍺", "beers": "🍻", "wine_glass": "🍷",
	"cocktail": "🍸", "champagne": "🍾", "clinking_glasses": "🥂",
}

var emojiByChar = func() map[string]string {
	names := make([]string, 0, len(emojiNames))
	for name := range emojiNames {
		names = append(names, name)
	}
	sort.Strings(names)

	byChar := map[string]string{}
	for _, name := range names {
		if _, known := byChar[emojiNames[name]]; !known {
			byChar[emojiNames[name]] = name
		}
	}
	return byChar
}()

// Emoji returns the character for an emoji name, ignoring any skin tone.
func Emoji(name string) (string, bool) {
	base, _, _ := strings.Cut(name, "::")
	char, ok := emojiNames[base]
	return char, ok
}

// EmojiName returns Slack's name for a character, or "" for an unknown one.
func EmojiName(char string) string {
	return emojiByChar[NormalizeEmoji(char)]
}

// NormalizeEmoji drops the variation selector, which only some sources include.
func NormalizeEmoji(char string) string {
	return strings.ReplaceAll(char, "️", "")
}

// Emojis returns every known character, sorted.
func Emojis() []string {
	chars := make([]string, 0, len(emojiByChar))
	for char := range emojiByChar {
		chars = append(chars, char)
	}
	sort.Strings(chars)
	return chars
}
