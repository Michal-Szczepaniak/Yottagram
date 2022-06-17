function getIcon(iconName) {
    var image = "image://theme/"
    switch (iconName) {
    case "Groups":
        image += "icon-m-users";
        break;
    case "Custom":
        image += "icon-m-file-folder";
        break;
    case "Setup":
        image += "icon-m-events";
        break;
    case "Favorite":
        image += "icon-m-favorite"
        break;
    case "Home":
        image += "icon-m-home";
        break;
    case "Game":
        image += "icon-m-game-controller";
        break;
    case "Mask":
        image += "icon-m-incognito";
        break;
    case "Travel":
        image += "icon-m-airplane-mode";
        break;
    case "Work":
        image += "icon-m-company";
        break;
    case "All":
        image += "icon-m-chat";
        break;
    case "Unread":
        image += "icon-m-message";
        break;
    case "Unmuted":
        image += "icon-m-browser-notifications";
        break;
    case "Bots":
        image = "qrc:/icons/icon-m-bots.png";
        break;
    case "Channels":
        image = "qrc:/icons/icon-m-channels.png";
        break;
    case "Private":
        image = "qrc:/icons/icon-m-private.png";
        break;
    case "Crown":
        image = "qrc:/icons/icon-m-crown.png";
        break;
    case "Flower":
        image = "qrc:/icons/icon-m-flower.png";
        break;
    case "Love":
        image = "qrc:/icons/icon-m-love.png";
        break;
    case "Party":
        image = "qrc:/icons/icon-m-party.png";
        break;
    case "Sport":
        image = "qrc:/icons/icon-m-sport.png";
        break;
    case "Study":
        image = "qrc:/icons/icon-m-study.png";
        break;
    case "Trade":
        image = "qrc:/icons/icon-m-trade.png";
        break;
    case "Cat":
        image = "qrc:/icons/icon-m-cat.png";
        break;
    default:
        return "image://theme/icon-m-contact"
    }
    return image;
}
